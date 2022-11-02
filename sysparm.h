#ifndef SYSPARM_H
#define SYSPARM_H



#include <vector>
#include "global.h"

#define MAX_RF_DEVICE_NUM 1

using namespace std;

enum State_en
{
    STATE_SYSTEM_FAIL,
    STATE_SYSTEM_START,
    STATE_SYSTEM_INITIALIZING,
    STATE_SYSTEM_CALIBRATION,
    STATE_SYSTEM_MODE_SETTING,
    STATE_POWER_READYTOACTIVE,
    STATE_POWER_ACTIVE,
    STATE_POWER_STOP,
    STATE_ERROR_OVER_TEMPERATURE,
    STATE_ERROR_OVER_IMPEDANCE,
    STATE_LAST

};

typedef struct tagRfFactors
{
    uint8 RfDeivceID; //RF Device ID

    uint8 tmMRFAState;
    uint8 tmAmplifierState;
    uint8 tmLastError;

    uint16 tmReadADCVoltage;
    uint16 tmReadADCCurrent;
    uint16 tmReadADCTemperatureA;
    uint16 tmReadADCTemperatureB;

    uint16 tmDACValueToSetLast;
    uint16 tmDACValueToSetforCal;


    uint16	tmRFonTime;

    uint16 rfPowerLevel_Init;
    uint16 rfPowerLevel;
    uint16 rfStepPowerLevel;
    uint16 rfTempLevel;



    uint16 rfPowerMeasured;
    uint16 rfVoltageMeasured;
    uint16 rfCurrentMeasured;
    uint16 rfImpedanceMeasured;
    uint16 rfTemperatureMeasuredA;
    uint16 rfTemperatureMeasuredB;
    uint16 rfTemperaturePreviousA;
    uint16 rfTemperaturePreviousB;

    uint16 tmTooHighImpedanceDelayTime;
    uint16 tmTooHighTemperatureDelayTime;

    uint32 tmMinImpedance;
    //U32 tmImpedanceFactor;


    uint32 tmTotalJ;
    uint32 tmAccumulation;

    //bool  tmAmplifierActive;      //RF amplifer enable
    bool System_result;
    bool tmRollOff;
    bool isStepwise;
    bool tmTooHighTempeatureFlag;
    bool tmTooHighImpedanceFlag;

//->->->->->->->->->->->->->->->->->->->->->->-> VIVA BD ->->->->->->->->->->->->->->->->->->->->->->->//

    uint16 combo_CheckSafetyCount;
//<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<- VIVA BD <-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-//

}RF_DEVICE_FACTORS;


class SysParm
{
public:
    const uint8 *SERIAL_NO;
    const uint8 *SW_VER;

    //en_RFDeviceOPMode 		tmOperationMode;

    uint8      tmImpedanceControlMethod;

    uint16 tmRFPowerLevelPerModes[3] = {30, 100, 30};     //venistar, continuance, temperature //Watt
    uint16 tmRFPowerLevelExtPerModes[3] = {0, 0, 120};  //venistar, continuance, temperature //Temp
    int32 tmRFCogluationTimePerModes[3] = {25, 0, 25}; //Sec
     int32 tmRFCogluationTimeMPerModes[3] = {0, 1, 0}; // Time Minit add
     uint32 tmRFCgluationTimesPerModes[3] = {0,0,0};
//    U32 tmRFSteptimePerModes[RF_DEVICE_OP_MODE_SIZE];

    uint32 tmElapsedTime;
    uint32 tmCutoff = 0;
    int32 tmRFCogluationTime;
    int32 tmRFCogluationTimeM;
    int32 modeSettingParam;


    bool isDoTestforCal;

    RF_DEVICE_FACTORS *mRfDeviceFactors;
    vector <uint16> vRFPowerToDAC;
    vector <uint16> vRFVoltage;
    vector <uint16> vRFCurrent;
    vector <uint16> vTemperatureA;
    vector <uint16> vTemperatureB;

public:
    SysParm();
    bool Inspect_table();
    ~SysParm();

    uint16 GetLastADVoltage(void) const {return lastADVoltage;}
    uint16 GetLastADCurrent(void) const {return lastADCurrent;}
    uint16 GetLastADTemperatureA(void) const {return lastADTemperatureA;}
    uint16 GetLastADTemperatureB(void) const {return lastADTemperatureB;}
    uint16 GetLastDACPower(void) const {return lastDACPower;}

    uint16 getLimitPower(void) const;
    uint16 getMinPower(void) const;
    uint16 GetWattToDACValuefor1Watt(void) const;

    bool doTestforCal(void) const {return isDoTestforCal;}

private:
    uint16 lastADVoltage;
    uint16 lastADCurrent;
    uint16 lastADTemperatureA;
    uint16 lastADTemperatureB;
    uint16 lastADTemperatureBoard;
    uint16 lastDACPower;
    uint16 limitPower;
    uint16 minPower;
    uint16 dacValuefor1Watt;




};

#endif // SYSPARM_H
