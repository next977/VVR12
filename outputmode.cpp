#include "outputmode.h"
#include "gpiotools.h"

using namespace std;

OutputMode::OutputMode(SysParm *sysParm)
{
    this->sysParm = sysParm;
    this->mRfDeviceFactors = sysParm->mRfDeviceFactors;
}

OutputMode::~OutputMode()
{

}


//void OutputMode::RFPowerControl()
//{
//    //cout << "OutputMode::RFPowerControl()" << endl;

//    if(proc->mRfDeviceFactors->rfPowerLevel == 0)
//    {
//       proc->DACPortOutputInc(0);
//    }
//    else if( proc->mRfDeviceFactors->rfPowerLevel > POWER_COMPENSATION &&
//             proc->mRfDeviceFactors->rfPowerMeasured/100 < proc->mRfDeviceFactors->rfPowerLevel- POWER_COMPENSATION)
//    {
//#if 0
//        if(proc->mRfDeviceFactors->tmDACValueToSetLast <= LIMIT_POWER)
//        {
//            proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_4);
//        }
//        else if(proc->mRfDeviceFactors->tmDACValueToSetLast > LIMIT_POWER)
//        {
//            proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
//        }
//#endif
//        proc->DACPortOutputInc(MRFA_AMPLIFIER_PARAM_2);
//    }
//    else if( proc->mRfDeviceFactors->rfPowerMeasured/100 > proc->mRfDeviceFactors->rfPowerLevel+POWER_COMPENSATION )
//    {
//        proc->DACPortOutputDec(MRFA_AMPLIFIER_PARAM_1);
//    }
//}

void OutputMode::MRFA_DoTestImpedanceState()
{

    if( mRfDeviceFactors->rfPowerLevel > 0 //change 5W->30W->15W(080131)->5W(101220)
            && mRfDeviceFactors->tmRFonTime > 1)//기본적으로 Power 값이 5이상으로 나오기 위해서는 2초 이하의 시간이 걸린다.
    {

        if( mRfDeviceFactors->tmMinImpedance  > mRfDeviceFactors->rfImpedanceMeasured )
        {
            mRfDeviceFactors->tmMinImpedance  = mRfDeviceFactors->rfImpedanceMeasured;
            return;
        }

        if(mRfDeviceFactors->tmMinImpedance < 50)
        {
            smTestImpedanceValueToChange = 30;
            tmImpedanceFactor = 0;
        }
        else
        {

            smTestImpedanceValueToChange = static_cast<uint32>(mRfDeviceFactors->tmMinImpedance* tmImpedanceChangeRatio/100);//modified(080131)
            tmImpedanceFactor = (mRfDeviceFactors->rfImpedanceMeasured < mRfDeviceFactors->tmMinImpedance) ? 0 : (mRfDeviceFactors->rfImpedanceMeasured - mRfDeviceFactors->tmMinImpedance);
        }
    }

}

void OutputMode::SafetyDutyCycle()
{
    
}

void OutputMode::RFPowerControlforCal()
{

}


void OutputMode::DACPortOutputInc(uint16 value)
{


    if(mRfDeviceFactors->tmDACValueToSetLast + value > sysParm->GetLastDACPower())
    {
        mRfDeviceFactors->tmDACValueToSetLast = sysParm->GetLastDACPower();
    }
    else
    {
        mRfDeviceFactors->tmDACValueToSetLast += value;
    }

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);

}

void OutputMode::DACPortOutputDec(uint16 value)
{


    if(value == 0 || mRfDeviceFactors->tmDACValueToSetLast < value)
    {
        mRfDeviceFactors->tmDACValueToSetLast = 0;
    }
    else
    {
        mRfDeviceFactors->tmDACValueToSetLast -= value;
    }

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
}

void OutputMode::DACPortOutputSet(uint16 value)
{
    mRfDeviceFactors->tmDACValueToSetLast = value;

    gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
}
