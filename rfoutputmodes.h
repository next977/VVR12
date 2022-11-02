#ifndef RFOUTPUTMODES_H
#define RFOUTPUTMODES_H

#include "outputmode.h"
#include "sysparm.h"

#ifndef __FEATURE_VVR12_BOARD__
class AutoMode : public OutputMode
{

public:
    explicit AutoMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~AutoMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_AUTO; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_AUTO; }
    void MRFA_DoTestImpedanceState() override;
};
#endif
class VenistarMode : public OutputMode
{
public:
    explicit VenistarMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~VenistarMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_VENISTAR; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_MANUAL; }
};

class ContinuanceMode : public OutputMode
{
private:
    uint32 Cutoff_Time;
public:
    explicit ContinuanceMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~ContinuanceMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    void SafetyDutyCycle() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_CONTINUANCE; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_MANUAL; }
    void MRFA_DoTestImpedanceState() override;
};

class TemperatureMode : public OutputMode
{
private:
    bool temp_con_RF_ON;
    uint16 currentTemp;
public:
    explicit TemperatureMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~TemperatureMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_TEMPERATURE; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_MANUAL; }
};

#endif // RFOUTPUTMODES_H
