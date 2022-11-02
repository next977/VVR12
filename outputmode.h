#ifndef OUTPUTMODE_H
#define OUTPUTMODE_H

#include "sysparm.h"

class OutputMode
{

/*
 *  1. Sensing
// *  2. Control wattRF_STATE_STOP
 *  3. check rf output
 *
 */

public:

    static const uint16 MRFA_AMPLIFIER_PARAM_4 = 4;
    static const uint16 MRFA_AMPLIFIER_PARAM_3 = 3;
    static const uint16 MRFA_AMPLIFIER_PARAM_2 = 2;
    static const uint16 MRFA_AMPLIFIER_PARAM_1 = 1;
    static const uint16 POWER_COMPENSATION = 1;
    static const uint32 tmImpedanceChangeRatio = 60;

    enum RF_State_en
    {
        RF_STATE_READY,
        RF_STATE_ACTIVE,
        RF_STATE_STOP,
        RF_STATE_IMPEDANCE_MONITORING
    };

    enum RF_Output_Mode_en
    {
        RF_DEVICE_OP_MODE_VENISTAR = 0,
        RF_DEVICE_OP_MODE_CONTINUANCE,
        RF_DEVICE_OP_MODE_TEMPERATURE,
        RF_DEVICE_OP_MODE_SIZE,
    };
#ifndef __FEATURE_VVR12_BOARD__
    enum RF_Output_CM_Mode_en
    {
        RF_DEVICE_OP_MODE_3CM = 0,
        RF_DEVICE_OP_MODE_7CM
    };
#endif

    enum RF_Imp_Mode_en
    {
        RF_DEVICE_IMP_MODE_MANUAL,
        RF_DEVICE_IMP_MODE_AUTO,
        RF_DEVICE_IMP_MODE_LAST
    };
    bool Up = false;
    bool Down = false;
    uint16 Interbal_Monitoring = 9;
    uint16 swing_flag = 0;
    uint16 swing_count = 0;
    explicit OutputMode(SysParm *sysParm);

    virtual ~OutputMode();

    virtual void RFStart() = 0;        //=0 자식 클래스에서 무조건 함수를 선언
    virtual void RFStop() = 0;
    virtual RF_Output_Mode_en getMode() = 0;
    #ifndef __FEATURE_VVR12_BOARD__
    virtual RF_Output_CM_Mode_en getCMMode() = 0;
    #endif

    virtual RF_Imp_Mode_en getImpedanceControlMethod() = 0;
    virtual void RFPowerControl() = 0;
    virtual void MRFA_DoTestImpedanceState();
    virtual void SafetyDutyCycle();
    virtual const int8* getModeTitle() = 0;

    void RFPowerControlforCal();
    void DACPortOutputInc(uint16 value);
    void DACPortOutputDec(uint16 value);
    void DACPortOutputSet(uint16 value);

    SysParm *sysParm;
    RF_DEVICE_FACTORS *mRfDeviceFactors;

    uint32 tmImpedanceFactor;
    uint32 smTestImpedanceValueToChange;

};

#endif // OUTPUTMODE_H
