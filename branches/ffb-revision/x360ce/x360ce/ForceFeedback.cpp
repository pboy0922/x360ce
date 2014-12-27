#include "stdafx.h"

#include "Common.h"
#include "InputHook.h"
#include "Config.h"

#include "ControllerManager.h"
#include "Controller.h"
#include "ForceFeedback.h"

ForceFeedback::ForceFeedback(Controller* pController) :
m_pController(pController)
{
    m_ForcePercent = 1.0f;
}

ForceFeedback::~ForceFeedback() {}

void ForceFeedback::Shutdown()
{
    if (m_pController->m_pDevice)
        m_pController->m_pDevice->SendForceFeedbackCommand(DISFFC_RESET);

    if (m_LeftMotor.effect) m_LeftMotor.effect->Release();
    if (m_RightMotor.effect) m_RightMotor.effect->Release();
}

DWORD ForceFeedback::SetState(XINPUT_VIBRATION* pVibration)
{
    if (ControllerManager::Get().XInputEnabled())
    {
        SetEffects(m_LeftMotor, pVibration->wLeftMotorSpeed);
        SetEffects(m_RightMotor, pVibration->wRightMotorSpeed);
    }

    return ERROR_SUCCESS;
}

BOOL CALLBACK ForceFeedback::EnumActuatorsCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID pvRef)
{
    ForceFeedback* ffb = (ForceFeedback*)pvRef;

    if ((pdidoi->dwFlags & DIDOI_FFACTUATOR) != 0)
    {
        PrintLog("Found actuator: '%s', ID: %u", pdidoi->tszName, DIDFT_GETINSTANCE(pdidoi->dwType));
        ffb->m_Actuators.push_back(pdidoi->dwType);
    }

    return DIENUM_CONTINUE;
}

BOOL CALLBACK ForceFeedback::EnumEffectsCallback(LPCDIEFFECTINFO pdiei, LPVOID pvRef)
{
    Caps caps;
    caps.ConstantForce = DIEFT_GETTYPE(pdiei->dwEffType) == DIEFT_CONSTANTFORCE;
    caps.PeriodicForce = DIEFT_GETTYPE(pdiei->dwEffType) == DIEFT_PERIODIC;
    PrintLog("ForceFeedback effect '%s'. IsConstant = %d, IsPeriodic = %d", pdiei->tszName, caps.ConstantForce, caps.PeriodicForce);

    ForceFeedback* ffb = (ForceFeedback*)pvRef;
    ffb->m_Caps.ConstantForce |= caps.ConstantForce;
    ffb->m_Caps.PeriodicForce |= caps.PeriodicForce;

    return DIENUM_CONTINUE;
}

bool ForceFeedback::IsSupported()
{
    DIDEVCAPS deviceCaps;
    deviceCaps.dwSize = sizeof(DIDEVCAPS);

    HRESULT hr = m_pController->m_pDevice->GetCapabilities(&deviceCaps);
    if (hr != DI_OK)
    {
        PrintLog("[PAD%d] IsForceSupported: GetCapabilities returned HR = %X", m_pController->m_user + 1, hr);
        return false;
    }

    bool ffSupported = ((deviceCaps.dwFlags & DIDC_FORCEFEEDBACK) == DIDC_FORCEFEEDBACK);
    PrintLog("[PAD%d] IsForceSupported: 0x%08X %s", m_pController->m_user + 1, deviceCaps.dwFlags, ffSupported == true ? "YES" : "NO");
    if (ffSupported)
    {
        m_Actuators.clear();
        hr = m_pController->m_pDevice->EnumObjects(EnumActuatorsCallback, this, DIDFT_AXIS);
        if (FAILED(hr)) PrintLog("[PAD%d] EnumActuatorsCallback failed with code HR = %X", m_pController->m_user + 1, hr);

        if (m_Actuators.size() > 0)
        {
            m_pController->m_pDevice->EnumEffects(EnumEffectsCallback, this, DIEFT_ALL);
            m_pController->m_pDevice->SendForceFeedbackCommand(DISFFC_RESET);
            m_pController->m_pDevice->SendForceFeedbackCommand(DISFFC_SETACTUATORSON);
            return true;
        }
    }

    return false;
}

bool ForceFeedback::SetEffects(Motor& motor, LONG speed)
{
    DWORD actuator = 0;
    for (auto itr = m_Actuators.begin(); itr != m_Actuators.end(); itr++)
        if (DIDFT_GETINSTANCE(*itr) == motor.actuator)
        {
            actuator = *itr;
            break;
        }
    if (!actuator) return false;

    LONG lDirection = 0;
    LONG force = (LONG)((float)speed / 65535 * DI_FFNOMINALMAX * m_ForcePercent);
    force = clamp(force, 0, DI_FFNOMINALMAX);

    DIEFFECT effectType;
    ZeroMemory(&effectType, sizeof(DIEFFECT));
    effectType.dwSize = sizeof(DIEFFECT);
    effectType.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTIDS;
    effectType.dwDuration = INFINITE;
    effectType.dwSamplePeriod = 0;
    effectType.dwGain = (DWORD)(motor.strength * DI_FFNOMINALMAX);
    effectType.dwTriggerButton = DIEB_NOTRIGGER;
    effectType.dwTriggerRepeatInterval = 0;
    effectType.cAxes = 1;
    effectType.rgdwAxes = &actuator;
    effectType.rglDirection = &lDirection;
    effectType.lpEnvelope = NULL;
    effectType.dwStartDelay = 0;
    effectType.lpvTypeSpecificParams = nullptr;

    GUID effectGUID;
    DICONSTANTFORCE contantForce;
    DIPERIODIC periodicForce;

    switch (motor.type)
    {
    case 1:
        if (!m_Caps.PeriodicForce) return false;
        effectGUID = GUID_Sine;
        effectType.cbTypeSpecificParams = sizeof(DIPERIODIC);
        effectType.lpvTypeSpecificParams = &periodicForce;
        periodicForce.dwPeriod = motor.period * 1000;
        periodicForce.dwMagnitude = (DWORD)force;
        periodicForce.dwPhase = 0;
        periodicForce.lOffset = 1000;
        break;
    case 2:
        if (!m_Caps.PeriodicForce) return false;
        effectGUID = GUID_SawtoothDown;
        effectType.cbTypeSpecificParams = sizeof(DIPERIODIC);
        effectType.lpvTypeSpecificParams = &periodicForce;
        periodicForce.dwPeriod = motor.period * 1000;
        periodicForce.dwMagnitude = (DWORD)force;
        periodicForce.dwPhase = 18000;
        periodicForce.lOffset = 1000;
        break;
    default:
        if (!m_Caps.ConstantForce) return false;
        effectGUID = GUID_ConstantForce;
        effectType.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
        effectType.lpvTypeSpecificParams = &contantForce;
        contantForce.lMagnitude = force;
        break;
    }

    HRESULT hr;
    if (!motor.effect)
    {
        hr = m_pController->m_pDevice->CreateEffect(effectGUID, &effectType, &motor.effect, NULL);
        if (FAILED(hr))
        {
            motor.effect = nullptr;
            PrintLog("[PAD%d] CreateEffect failed with code HR = %X, FFBType = %u", m_pController->m_user + 1, hr, motor.type);
            return false;
        }
    }

    if (force == 0) motor.effect->Stop();
    else
    {
        u32 flags = DIEP_START | DIEP_GAIN | DIEP_TYPESPECIFICPARAMS;
        hr = motor.effect->SetParameters(&effectType, flags);
        if (FAILED(hr))
        {
            PrintLog("[PAD%d] SetParameters failed with code HR = %X, FFBType = %u", m_pController->m_user + 1, hr, motor.type);
            return false;
        }
    }

    return true;
}
