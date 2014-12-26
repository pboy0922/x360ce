#pragma once

#include <dinput.h>
#include "Config.h"
#include "Mutex.h"

struct ForceFeedbackCaps
{
    bool ConstantForce;
    bool PeriodicForce;

    ForceFeedbackCaps()
    {
        ConstantForce = false;
        PeriodicForce = false;
    }
};

struct ForceFeedbackMotor
{
    u8 type;
    u32 period;
    float strength;
    int actuator;
    LPDIRECTINPUTEFFECT effect;

    ForceFeedbackMotor()
    {
        type = 0;
        period = 0;
        strength = 1.0f;
        actuator = -1;
        effect = nullptr;
    }
};

class ForceFeedback
{
    friend class Controller;

public:
    ForceFeedback(Controller* pController);
    virtual ~ForceFeedback();

    void Shutdown();
    bool SetState(XINPUT_VIBRATION* pVibration);

    float m_ForcePercent;
    ForceFeedbackMotor m_LeftMotor;
    ForceFeedbackMotor m_RightMotor;

private:
    static BOOL CALLBACK EnumFFAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, LPVOID pvRef);
    static BOOL CALLBACK EnumEffectsCallback(LPCDIEFFECTINFO di, LPVOID pvRef);

    bool IsSupported();
    bool SetEffects(ForceFeedbackMotor& motor, LONG motorSpeed);

    Controller* m_pController;
    ForceFeedbackCaps m_Caps;
    std::vector<DWORD> m_Actuators;
};
