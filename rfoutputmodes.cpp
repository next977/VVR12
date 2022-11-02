#include "rfoutputmodes.h"
#include <string>
#include "QDebug"
/*
 *
 * AUTO MODE
 *
 *
using namespace std;


AutoMode::AutoMode(SysParm *sysParm) : OutputMode (sysParm)
{

}

AutoMode::~AutoMode()
{
}

const int8* AutoMode::getModeTitle()
{
    //return string("AUTO").c_str();
    return "AUTO";
}

void AutoMode::RFStart()
{

}

void AutoMode::RFStop()
{

}

void AutoMode::RFPowerControl()
{

}

void AutoMode::MRFA_DoTestImpedanceState()
{
    OutputMode::MRFA_DoTestImpedanceState();


    if(OutputMode::tmImpedanceFactor > OutputMode::smTestImpedanceValueToChange)
    {
        //State 변경에 카운터를 넣은 이유는 1~2,3 개의 쓰레기값을 보정 하기 위해.    overImpedance, overTemperature, rolloff
        //QTimer::singleShot(11 * 1000, this, SLOT(MRFA_ImpedanceControlTimeOut()));
    }

    //QTimer::singleShot(5 * 1000, this, SLOT(helloWave()));

}
*/
VenistarMode::VenistarMode(SysParm *sysParm)  : OutputMode (sysParm)
{

}

VenistarMode::~VenistarMode()
{


}

const int8* VenistarMode::getModeTitle()
{
    return "VENISTAR";
}

void VenistarMode::RFStart()
{

}
void VenistarMode::RFStop()
{

}
void VenistarMode::RFPowerControl()
{
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;
    mRfDeviceFactors->tmTooHighImpedanceDelayTime = sysParm->tmElapsedTime;
    mRfDeviceFactors->tmTooHighTemperatureDelayTime = sysParm->tmElapsedTime;
    if(mRfDeviceFactors->rfPowerLevel == 0)
    {
        //DACPortOutputSet(0);
        DACPortOutputSet(100);
    }
    else if( mRfDeviceFactors->rfPowerLevel > POWER_COMPENSATION &&
             mRfDeviceFactors->rfPowerMeasured/100 < mRfDeviceFactors->rfPowerLevel- POWER_COMPENSATION)
    {
#if 0
        if(proc->mRfDeviceFactors->tmDACValueToSetLast <= LIMIT_POWER)
        {
            proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_4);
        }
        else if(proc->mRfDeviceFactors->tmDACValueToSetLast > LIMIT_POWER)
        {
            proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
        }
#endif
        DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
    }
    else if( mRfDeviceFactors->rfPowerMeasured/100 > mRfDeviceFactors->rfPowerLevel+POWER_COMPENSATION )
    {
        DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);
    }

}
/*
 *
 * CONTINUANCE MODE
 *
 * */

ContinuanceMode::ContinuanceMode(SysParm *sysParm)  : OutputMode (sysParm)
{

}

ContinuanceMode::~ContinuanceMode()
{


}

const int8* ContinuanceMode::getModeTitle()
{
    return "CONTINUANCE";
}

void ContinuanceMode::RFStart()
{
    Cutoff_Time = 0;

}

void ContinuanceMode::RFStop()
{

}
void ContinuanceMode::SafetyDutyCycle()
{
    if(sysParm->tmElapsedTime - Cutoff_Time >= 30)
    {
        sysParm->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_ACTIVE; 
        Cutoff_Time = sysParm->tmElapsedTime;
    }
}
void ContinuanceMode::MRFA_DoTestImpedanceState()
{
    OutputMode::MRFA_DoTestImpedanceState();
    if(sysParm->tmElapsedTime - Cutoff_Time >= 10)
    {
       sysParm->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_READY;   
       Cutoff_Time = sysParm->tmElapsedTime;
       sysParm->tmCutoff = sysParm->tmElapsedTime - Cutoff_Time;
    }
}

void ContinuanceMode::RFPowerControl()
{
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;
    mRfDeviceFactors->tmTooHighImpedanceDelayTime = sysParm->tmElapsedTime;
    mRfDeviceFactors->tmTooHighTemperatureDelayTime = sysParm->tmElapsedTime;
        if(mRfDeviceFactors->rfPowerLevel == 0)
        {
            //DACPortOutputInc(0);
            DACPortOutputSet(100);
        }
        else if( mRfDeviceFactors->rfPowerLevel > POWER_COMPENSATION &&
                 mRfDeviceFactors->rfPowerMeasured/100 < mRfDeviceFactors->rfPowerLevel- POWER_COMPENSATION)
        {
    #if 0
            if(proc->mRfDeviceFactors->tmDACValueToSetLast <= LIMIT_POWER)
            {
                proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_4);
            }
            else if(proc->mRfDeviceFactors->tmDACValueToSetLast > LIMIT_POWER)
            {
                proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
            }
    #endif
            DACPortOutputInc(5);
        }
        else if( mRfDeviceFactors->rfPowerMeasured/100 > mRfDeviceFactors->rfPowerLevel+POWER_COMPENSATION )
        {
            DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);
        }
}
/*
 *
 * TEMPERATURE MODE
 *
 * */
TemperatureMode::TemperatureMode(SysParm *sysParm)  : OutputMode (sysParm)
{

}

TemperatureMode::~TemperatureMode()
{


}

const int8* TemperatureMode::getModeTitle()
{
    return "TEMPERATURE";
}

void TemperatureMode::RFStart()
{
    temp_con_RF_ON = true;

}

void TemperatureMode::RFStop()
{


}

void TemperatureMode::RFPowerControl()
{

                //+++++++++++++++++++++++++ 온도제어 보정 (5월 14일. 2014) +++++++++++++++++++++++++//
    uint8 Temperature_low_limit;
    uint8 Temperature_lowPower_limit = 1;
    mRfDeviceFactors->tmTooHighImpedanceDelayTime = sysParm->tmElapsedTime;
    mRfDeviceFactors->tmTooHighTemperatureDelayTime = sysParm->tmElapsedTime;
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;

    if (temp_con_RF_ON == true)	//((sysparam.temp_con_RF_ON == 0)&&(mRfDeviceFactors[0].rfTempLevel>=50))
    {
        Temperature_low_limit = 1;//2;	//3;
    }
    else
    {
        Temperature_low_limit = 1;
    }


    if(mRfDeviceFactors->rfPowerLevel == 0)
    {
        //DACPortOutputSet(0);
        DACPortOutputSet(100);
    }
#if 0
    else if(mRfDeviceFactors->rfTemperatureMeasuredA/10 < mRfDeviceFactors->rfTempLevel - Temperature_low_limit)
    {
#ifdef __VVR_SIMULATION__
            mRfDeviceFactors[0].tmReadADCTemperature+=15;
#endif


            if((mRfDeviceFactors->rfPowerLevel > POWER_COMPENSATION) && (mRfDeviceFactors->rfPowerMeasured/100 < mRfDeviceFactors->rfPowerLevel- POWER_COMPENSATION))
            {
                if(mRfDeviceFactors->tmDACValueToSetLast <= sysParm->getLimitPower())
                {
                    DACPortOutputInc(MRFA_AMPLIFIER_PARAM_4);
                }
                if(mRfDeviceFactors->tmDACValueToSetLast > sysParm->getLimitPower())
                {
                    DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
                }
            }
            else if( mRfDeviceFactors->rfPowerMeasured/100 > mRfDeviceFactors[i].rfPowerLevel+1)
            {
                DACPortOutputDec(MRFA_AMPLIFIER_PARAM_3);
            }
    }
    else if((mRfDeviceFactors->rfTemperatureMeasuredA/10) >= mRfDeviceFactors->rfTempLevel - Temperature_low_limit)
    {
        #ifdef __VVR_SIMULATION__
        mRfDeviceFactors[0].tmReadADCTemperature-=1;
        #endif

        if(mRfDeviceFactors->rfPowerMeasured/100 > 2)
        {
            DACPortOutputSet(sysParm->GetWattToDACValuefor1Watt());
        }

        if( mRfDeviceFactors->rfCurrentMeasured < (100-20) )
        {
            DACPortOutputInc(MRFA_AMPLIFIER_PARAM_3);
        }
        if( mRfDeviceFactors->rfCurrentMeasured > (100+10) )//(current+20) )
        {
            DACPortOutputDec(MRFA_AMPLIFIER_PARAM_2);
        }

        mRfDeviceFactors->tmRFonTime=0;waitTime

//        if((VASCAB_TempFactors[0].rfTemperatureMeasured/10) >= mRfDeviceFactors[0].rfTempLevel)
//        {
//            temp_con_RF_ON = false;
//        }
    }
#endif
    else if(swing_flag == 0)
    {
        if(mRfDeviceFactors->rfPowerMeasured/100 < mRfDeviceFactors->rfPowerLevel)
        {
            DACPortOutputInc(MRFA_AMPLIFIER_PARAM_3);
        }
        if(mRfDeviceFactors->rfTemperatureMeasuredB/10 > mRfDeviceFactors->rfTempLevel - 5)
        {
            swing_flag = 1;
        }
        currentTemp = mRfDeviceFactors->rfTemperatureMeasuredB;
    }
    else if(swing_flag == 1 || swing_flag == 2)
    {

        if(swing_flag == 1)
        {
            if( currentTemp <= mRfDeviceFactors->rfTemperatureMeasuredB)
            {
                currentTemp = mRfDeviceFactors->rfTemperatureMeasuredB;
                if(sysParm->mRfDeviceFactors->tmDACValueToSetLast >= 200) //sysParm->mRfDeviceFactors->tmDACValueToSetLast > 5
                {
                    DACPortOutputDec(5);
                }

            }
            else if(currentTemp > mRfDeviceFactors->rfTempLevel)
            {
                qDebug("OK");
                swing_flag = 2;
            }
            else if(currentTemp <= mRfDeviceFactors->rfTempLevel)
            {
                qDebug("Fail");
                swing_flag = 3;
            }
        }
        else
        {
            if((mRfDeviceFactors->rfTemperatureMeasuredB/10 > mRfDeviceFactors->rfTempLevel)&&(sysParm->mRfDeviceFactors->tmDACValueToSetLast >= 200)) //sysParm->mRfDeviceFactors->tmDACValueToSetLast > 5
            {
                DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);
            }
            else
            {
                swing_flag = 3;
            }
        }
    }
    else if(swing_flag == 3)
    {

        Interbal_Monitoring = 23;
        if((mRfDeviceFactors->rfTemperatureMeasuredB/10) < mRfDeviceFactors->rfTempLevel - Temperature_low_limit)
        {
    #ifdef __VVR_SIMULATION__
                mRfDeviceFactors[0].tmReadADCTemperature+=15;
    #endif
                if((mRfDeviceFactors->rfPowerLevel > POWER_COMPENSATION) && (mRfDeviceFactors->rfPowerMeasured/100 < mRfDeviceFactors->rfPowerLevel- POWER_COMPENSATION))
                {
                    DACPortOutputInc(MRFA_AMPLIFIER_PARAM_1);
                }
    #if 0
                else if( mRfDeviceFactors->rfPowerMeasured/100 > mRfDeviceFactors[i].rfPowerLevel+1)
                {
                    DACPortOutputDec(MRFA_AMPLIFIER_PARRF_Start_State = true;AM_3);
                }
    #endif
        }
        else if((mRfDeviceFactors->rfTemperatureMeasuredB/10) > mRfDeviceFactors->rfTempLevel + Temperature_low_limit)
        {
            #ifdef __VVR_SIMULATION__
            mRfDeviceFactors[0].tmReadADCTemperature-=1;
            #endif
    #if 0
            if(mRfDeviceFactors->rfPowerMeasured/100 > 2)
            {
                DACPortOutputSet(sysParm->GetWattToDACValuefor1Watt());
            }

            if( mRfDeviceFactors->rfCurrentMeasured < (100-20) )
            {
                DACPortOutputInc(MRFA_AMPLIFIER_PARAM_3);
            }
            if( mRfDeviceFactors->rfCurrentMeasured > (100+10) )//(current+20) )
            {
                DACPortOutputDec(MRFA_AMPLIFIER_PARAM_2);
            }
    #else
            if((mRfDeviceFactors->rfPowerMeasured/100) > mRfDeviceFactors->rfPowerLevel + POWER_COMPENSATION)
            {
                DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);
    #ifndef __VVR12_BOARD_TEST__
                if( mRfDeviceFactors->tmDACValueToSetLast <= 500)
                {
                    mRfDeviceFactors->tmDACValueToSetLast = 500;
                }
                /*
                else if (mRfDeviceFactors->tmReadADCTemperatureB >= 58112)
                {
                    mRfDeviceFactors->tmDACValueToSetLast = 600;
                }
                */
    #endif
            }
            else if(sysParm->mRfDeviceFactors->tmDACValueToSetLast >= 200) // sysParm->mRfDeviceFactors->tmDACValueToSetLast >= 5
            {
                DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);


    #ifndef __VVR12_BOARD_TEST__
                if( mRfDeviceFactors->tmDACValueToSetLast <= 500)
                {
                    mRfDeviceFactors->tmDACValueToSetLast = 500;
                }
    #endif
            }

            mRfDeviceFactors->tmRFonTime=0;

    //        if((VASCAB_TempFactors[0].rfTemperatureMeasured/10) >= mRfDeviceFactors[0].rfTempLevel)
    //        {
    //            temp_con_RF_ON = false;
    //        }
        }
    #endif
    }


}
