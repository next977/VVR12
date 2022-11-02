#include "sysparm.h"
#include "mrfa_rf_table.h"
#include <QDebug>

extern const uint8 SERIAL_NO[];
extern const uint8 SW_VER[];

SysParm::SysParm()
{
    mRfDeviceFactors = new RF_DEVICE_FACTORS;

    mRfDeviceFactors->tmReadADCCurrent = 0;
    mRfDeviceFactors->tmReadADCVoltage = 0;
    mRfDeviceFactors->tmReadADCTemperatureA = 0;
    mRfDeviceFactors->tmReadADCTemperatureB = 0;
    mRfDeviceFactors->rfPowerLevel = 0;


    mRfDeviceFactors->tmDACValueToSetLast = 0;
    mRfDeviceFactors->tmDACValueToSetforCal = 0;



    tmElapsedTime= 0;
    isDoTestforCal = false;


//    this->SERIAL_NO = (const U8*)&SERIAL_NO[0];
//    this->SW_VER = (const U8*)&SW_VER[0];



    Inspect_table();

    limitPower = vRFPowerToDAC.at(TABLE_STEP_RFPOWER_REF-11);
    minPower = vRFPowerToDAC.at(14);

    for(uint32 i=0; i<TABLE_STEP_RFPOWER_REF; i++)
    {
        if(vRFPowerToDAC.at(TABLE_STEP_RFPOWER_REF+i) >= 10 )
        {
            dacValuefor1Watt = vRFPowerToDAC.at(i);
            break;
        }
    }



}

SysParm::~SysParm()
{
    if(mRfDeviceFactors)
        delete mRfDeviceFactors;
}

uint16 SysParm::getMinPower() const
{
    return minPower;
}

uint16 SysParm::GetWattToDACValuefor1Watt() const
{
    return dacValuefor1Watt;
}

uint16 SysParm::getLimitPower() const
{
    return limitPower;
}



bool SysParm::Inspect_table()
{
    uint16 v1, v2, v3, v4, v5, i;

    v1 = v2 = v3 = v4 = v5 = 0;

    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  TransTableRFPowerToDAC[i][0].tmDigitalValue)
        {
            qCritical("error TransTableRFPowerToDAC");
            return false;
        }
        v1 = TransTableRFPowerToDAC[i][0].tmDigitalValue;
        vRFPowerToDAC.push_back(v1);
    }
    lastDACPower = static_cast<uint16>(v1);

    v1 = 0;
    for (i = 0; i < TABLE_STEP_RFPOWER_REF; i++)
    {
        if(v1 >  TransTableRFPowerToDAC[i][0].tmRealValue)
        {
            qCritical("error TransTableRFPowerToDAC");
            return false;
        }
        v1 = TransTableRFPowerToDAC[i][0].tmRealValue;
        vRFPowerToDAC.push_back(v1);
    }



    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  TransTableRFVoltage[i][0].tmDigitalValue)
        {
            qCritical("error TransTableRFVoltage");
            return false;
        }
        v2 = TransTableRFVoltage[i][0].tmDigitalValue;
        vRFVoltage.push_back(v2);
    }
    lastADVoltage = static_cast<uint16>(v2);
    v2 = 0;
    for (i = 0; i < TABLE_STEP_VOLTAGE; i++)
    {
        if(v2 >  TransTableRFVoltage[i][0].tmRealValue)
        {
            qCritical("error TransTableRFVoltage");
            return false;
        }
        v2 = TransTableRFVoltage[i][0].tmRealValue;
        vRFVoltage.push_back(v2);
    }


    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  TransTableRFCurrent[i][0].tmDigitalValue)
        {
            qCritical("error TransTableRFCurrent");
            return false;
        }
        v3 = TransTableRFCurrent[i][0].tmDigitalValue;
        vRFCurrent.push_back(v3);
    }
    lastADCurrent = static_cast<uint16>(v3);
    v3=0;
    for (i = 0; i < TABLE_STEP_CURRENT; i++)
    {
        if(v3 >  TransTableRFCurrent[i][0].tmRealValue)
        {
            qCritical("error TransTableRFCurrent");
            return false;
        }
        v3 = TransTableRFCurrent[i][0].tmRealValue;
        vRFCurrent.push_back(v3);
    }


    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v4 >  TransTableTemperature[i][0].tmDigitalValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v4 = TransTableTemperature[i][0].tmDigitalValue;
        vTemperatureA.push_back(v4);
    }
    lastADTemperatureA = static_cast<uint16>(v4);
    v4=0;
    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v4 >  TransTableTemperature[i][0].tmRealValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v4 = TransTableTemperature[i][0].tmRealValue;
        vTemperatureA.push_back(v4);
    }


    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v5 >  TransTableTemperature[i][1].tmDigitalValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v5 = TransTableTemperature[i][1].tmDigitalValue;
        vTemperatureB.push_back(v5);
    }
    lastADTemperatureB = static_cast<uint16>(v5);
    v5=0;
    for (i = 0; i < TABLE_STEP_TEMPERATURE; i++)
    {
        if(v5 >  TransTableTemperature[i][1].tmRealValue)
        {
            qCritical("error TransTableTemperature");
            return false;
        }
        v5 = TransTableTemperature[i][1].tmRealValue;
        vTemperatureB.push_back(v5);
    }




    return true;
}
