#pragma once

#include <dinput.h>
#include "Config.h"
#include "Mutex.h"

class ForceFeedback
{
    friend class Controller;

    struct Caps
    {
        Caps()
        {
            ConstantForce = false;
            PeriodicForce = false;
        }

        bool ConstantForce;
        bool PeriodicForce;
    };

    struct Motor
    {
        Motor() {
            type = 0;
            period = 0;
            strength = 1.0f;
            actuator = -1;
            effect = nullptr;
            lastStarted = 0;
        }

        u8 type;
        u32 period;
        float strength;
        int actuator;
        LPDIRECTINPUTEFFECT effect;
        DWORD lastStarted;
    };

public:
    ForceFeedback(Controller* pController);
    ~ForceFeedback();

    void Shutdown();
    DWORD SetState(XINPUT_VIBRATION* pVibration);

    float m_ForcePercent;
    Motor m_LeftMotor;
    Motor m_RightMotor;

private:
    static BOOL CALLBACK EnumActuatorsCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID pvRef);
    static BOOL CALLBACK EnumEffectsCallback(LPCDIEFFECTINFO pdiei, LPVOID pvRef);

    bool IsSupported();
    bool SetEffects(Motor& motor, LONG speed);

    Controller* m_pController;
    Caps m_Caps;
    std::vector<DWORD> m_Actuators;
};