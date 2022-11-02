#include "outputthread.h"
#include "gpiotools.h"
#include "rfoutputmodes.h"
#include <QElapsedTimer>

using namespace gpioTools;


//#define __INTERVAL_MONITORING__ 23//9   //30ms, 33 cycle
#define __INTERVAL_TEST__ false

OutputThread::OutputThread(QObject *parent, SysParm *sysParm) : QObject (parent), WorkerThread ()
{

    //slot signals
#if __INTERVAL_TEST__
    checkpersec = new QTimer;
    counter = 0;
    connect(checkpersec, SIGNAL(timeout()), this, SLOT(slot_dispCounter()));
    checkpersec->start(1000);
#endif

    this->sysParm = sysParm;
    this->mRfDeviceFactors = sysParm->mRfDeviceFactors;

    monitoringTask = new MonitoringTask(this, sysParm);


    tmRFPowerActiveState = RF_ACTIVE_OFF;
    this->mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_READY;
    this->mRfDeviceFactors->tmDACValueToSetLast = 0;
#ifndef __FEATURE_VVR12_BOARD__
    vModes.push_back(new AutoMode(sysParm));
#endif
    vModes.push_back(new VenistarMode(sysParm));
    vModes.push_back(new ContinuanceMode(sysParm));
    vModes.push_back(new TemperatureMode(sysParm));

   //outputMode = vModes.at(OutputMode::RF_DEVICE_OP_MODE_VENISTAR);
    outputMode = vModes.at(OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE);

}




OutputThread::~OutputThread()
{
    for(vector<OutputMode *>::iterator iter = vModes.begin(); iter != vModes.end(); )
    {
        OutputMode *modes = *iter;
        delete modes;
        ++iter;
    }

    if(monitoringTask) delete monitoringTask;
}


void OutputThread::loop()
{

    QElapsedTimer _mTimer;
    QElapsedTimer _countTimer;
    _mTimer.start();
    _countTimer.start();
    /*
     *  Start call function.
     *  Synchronous call function 일반적인 처리는 동기로
     *  Sensing adc
     */
    monitoringTask->GetAmplifierParameters();

    //Calibration mode
    //Impedance check
    //rf output for mode
    if(tmRFPowerActiveState == RF_ACTIVE_ON)
    {
            if((sysParm->tmElapsedTime/60==sysParm->tmRFCogluationTimeM)&&(sysParm->tmElapsedTime%60==sysParm->tmRFCogluationTime))
            {
                        RFStop();
                        tmRFPowerActiveState = RF_ACTIVE_OFF;
                        outputMode->Up = false;
                        outputMode->Down = false;
                        outputMode->swing_count = 0;
                        outputMode->swing_flag = 0;
                        outputMode->Interbal_Monitoring = 9;
                        return;
            }
        //RF output
        switch(mRfDeviceFactors->tmMRFAState)
                {
                    case OutputMode::RF_STATE_READY:
                         //outputMode->DACPortOutputDec(0);
                         outputMode->DACPortOutputSet(100);
                         outputMode->SafetyDutyCycle();
                        break;

                    case OutputMode::RF_STATE_STOP:
                        if( mRfDeviceFactors->rfCurrentMeasured < (150-20) )
                        {
                            outputMode->DACPortOutputInc(OutputMode::MRFA_AMPLIFIER_PARAM_2);//MRFA_AMPLIFIER_PARAM_DAC_FAST_INCREMENT;
                        }
                        else if( mRfDeviceFactors->rfCurrentMeasured > (150+10) )//(current+20) )
                        {
                            outputMode->DACPortOutputDec(OutputMode::MRFA_AMPLIFIER_PARAM_2);
                        }

                        break;

                    case OutputMode::RF_STATE_IMPEDANCE_MONITORING:
                        if( mRfDeviceFactors->rfCurrentMeasured < (170-20) )
                        {
                            outputMode->DACPortOutputInc(OutputMode::MRFA_AMPLIFIER_PARAM_2);//MRFA_AMPLIFIER_PARAM_DAC_FAST_INCREMENT;
                        }
                        else if( mRfDeviceFactors->rfCurrentMeasured > (170+10) )//(current+20) )
                        {
                            outputMode->DACPortOutputDec(OutputMode::MRFA_AMPLIFIER_PARAM_2);
                        }
                        break;

                    case OutputMode::RF_STATE_ACTIVE:
                        outputMode->RFPowerControl();
                        outputMode->MRFA_DoTestImpedanceState();
                        break;

                }
        //RF output monitoring

        //calulator energy

        //warning
        if(mRfDeviceFactors->tmMRFAState == OutputMode::RF_STATE_ACTIVE )
        {
            CheckImpedanceState();
            CheckOverTemperatureState();
            CheckSafetyState();
        }
    }
        else if(sysParm->doTestforCal())
        {
            gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);
        }

    /*
     *  End call function.
     */

#if __INTERVAL_TEST__
    counter++;
#endif

//    waitTime = __INTERVAL_MONITORING__ - _mTimer.elapsed();    // 30ms 32 - 34 per 1 second to the pc
    waitTime = outputMode->Interbal_Monitoring - _mTimer.elapsed();
    //qDebug("%d",waitTime);
    if(waitTime >= 0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli> (waitTime));


    }
       // qDebug("%d",_countTimer.elapsed()); //outputMode->Interbal_Monitoring = 9  ----  9~10ms
}

void OutputThread::slot_dispCounter()
{
    qDebug("OutputThread run %d per 1seconds, waittime = %lf", counter, waitTime);
    counter = 0;
}


void OutputThread::SetOutputMode(OutputMode::RF_Output_Mode_en mode)
{
    if(tmRFPowerActiveState != RF_ACTIVE_ON)
    {

        if(mode == OutputMode::RF_DEVICE_OP_MODE_SIZE)
        {
            mode = outputMode->getMode();
            if(mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
            {
                mode = OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE;
            }
            else if(mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
            {
                mode = OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE;
            }
            else if(mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
            {
                mode = OutputMode::RF_DEVICE_OP_MODE_VENISTAR;
            }
        }

        outputMode = vModes.at(mode);
        emit sig_modeChange(mode);
    }
}

OutputMode::RF_Output_Mode_en OutputThread::GetOutputMode()
{
    return outputMode->getMode();
}

const int8* OutputThread::GetOutputModeTitle()
{
    return outputMode->getModeTitle();
}

void OutputThread::RFStart()
{
    setPaused(true);
    outputMode->DACPortOutputDec(0);
    //sysParm->tmElapsedTime = 0;
    qDebug("GetMode = %d",outputMode->getMode());
    qDebug("outputTread_RF_Start");

    mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_ACTIVE;

    mRfDeviceFactors->tmTotalJ = 0;

    mRfDeviceFactors->tmRollOff = false;
    mRfDeviceFactors->isStepwise = false;
    mRfDeviceFactors->tmTooHighImpedanceDelayTime = 0;
    mRfDeviceFactors->tmTooHighTemperatureDelayTime = 0;
    mRfDeviceFactors->tmTooHighTempeatureFlag = false;
    mRfDeviceFactors->tmTooHighImpedanceFlag = false;

    outputMode->RFStart();
    tmRFPowerActiveState = RF_ACTIVE_ON;

    //Timer Start
    btimer.start(500, this);
    rfTimer.restart();

    setPaused(false);

    sysParm->isDoTestforCal = true;
    EnableAmplifierPowerOutput();
    SetAmplifierPowerOutputDirection(RFOUTPUT_DIR_TARGET);
    if(outputMode->getMode() == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
    {
        SetElectrodeType(ELECTRODE_MONOPOLAR);
    }
    else if(outputMode->getMode() == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
    {
        if(Venistar_CM_mode == RF_DEVICE_OP_MODE_3CM)
        {
            SetElectrodeType(ELECTRODE_BI_3CM);
        }
        else
        {
            SetElectrodeType(ELECTRODE_BI_7CM);
        }
        qDebug("Venistar_CM_mode = %d",Venistar_CM_mode);
    }
    else if(outputMode->getMode() == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
    {
        if(Temperature_CM_mode == RF_DEVICE_OP_MODE_3CM)
        {
            SetElectrodeType(ELECTRODE_BI_3CM);
        }
        else
        {
            SetElectrodeType(ELECTRODE_BI_7CM);
        }
        qDebug("Temperature_CM_mode = %d",Temperature_CM_mode);
    }

    emit sig_rfStart();
}

void OutputThread::RFStop()
{

    outputMode->DACPortOutputDec(0);
    SetElectrodeType(ELECTRODE_NONE);
    SetAmplifierPowerOutputDirection(RFOUTPUT_DIR_DUMMY);//RFOUTPUT_DIR_TARGET
    DisableAmplifierPowerOutput();
    sysParm->isDoTestforCal = false;
    //Timer Stop
    btimer.stop();
    tmRFPowerActiveState = RF_ACTIVE_OFF;
    setPaused(true);
    outputMode->RFStop();
    mRfDeviceFactors->tmMRFAState = OutputMode::RF_STATE_READY;
    outputMode->swing_count = 0;
    outputMode->swing_flag = 0;
    outputMode->Interbal_Monitoring = 9;
    setPaused(false);
    emit sig_rfStop();
}



bool OutputThread::GetRFPowerActiveState() const
{
    return tmRFPowerActiveState;
}

void OutputThread::SetRFPowerActiveState(bool state)
{
    tmRFPowerActiveState = state;
}

void OutputThread::MRFA_InitRFADeviceState()
{

}

void OutputThread::MRFA_DeInitRFADeviceState()
{

}

void OutputThread::CheckImpedanceState()
{
    OutputMode::RF_Output_Mode_en Mode = GetOutputMode();
    //shot
    if(mRfDeviceFactors->tmReadADCCurrent > 50000)
    {
        qDebug("Short");
        RFStop();
        emit sig_overImpedance();
    }
    //low
    if(mRfDeviceFactors->rfPowerLevel != 0 &&
        mRfDeviceFactors->rfImpedanceMeasured <= 10 &&
        mRfDeviceFactors->rfCurrentMeasured/10 >= 200)
    {
         mRfDeviceFactors->tmTooHighImpedanceFlag = true;

            if(mRfDeviceFactors->tmTooHighImpedanceDelayTime > 0) //mRfDeviceFactors->tmTooHighImpedanceDelayTime > 0 && sysParm->tmCutoff > 2
            {
                 qDebug("Impedance Low Currunt over");
                 RFStop();
                 emit sig_lowImpedance();
            }

    }
    else if(mRfDeviceFactors->rfPowerLevel != 0 &&
        mRfDeviceFactors->rfImpedanceMeasured <= 10 &&
        mRfDeviceFactors->rfCurrentMeasured/10 > mRfDeviceFactors->rfVoltageMeasured/10)
    {
         mRfDeviceFactors->tmTooHighImpedanceFlag = true;

             if(mRfDeviceFactors->tmTooHighImpedanceDelayTime > 2  ) //mRfDeviceFactors->tmTooHighImpedanceDelayTime > 2 && sysParm->tmCutoff > 2
             {
                  qDebug("Impedance Low");
                  RFStop();
                  emit sig_lowImpedance();
             }

    }
    //over
    else if((mRfDeviceFactors->rfPowerLevel != 0 && mRfDeviceFactors->rfImpedanceMeasured >= 800) && ( ( mRfDeviceFactors->rfVoltageMeasured/10 > mRfDeviceFactors->rfCurrentMeasured/10 ) ||( (mRfDeviceFactors->rfVoltageMeasured/10 == 0) &&( mRfDeviceFactors->rfCurrentMeasured/10 == 0) ) )) //over ADD
    {
        mRfDeviceFactors->tmTooHighImpedanceFlag = true;
            if(mRfDeviceFactors->tmTooHighImpedanceDelayTime > 2 )
            {
                    qDebug("Impedance High");
                    RFStop();
                    emit sig_overImpedance();
            }

    }
    else
    {
        mRfDeviceFactors->tmTooHighImpedanceFlag = false;
       // mRfDeviceFactors->tmTooHighImpedanceDelayTime = 0;
    }
}
void OutputThread::CheckOverTemperatureState()
{
#if 0
    if(mRfDeviceFactors->rfTemperatureMeasuredA > 990 && mRfDeviceFactors->rfTemperaturePrevious > 960)
    {
        mRfDeviceFactors->tmTooHighTempeatureFlag = true;

        if(mRfDeviceFactors->tmTooHighTemperatureDelayTime > 2)
        {
            RFStop();
            emit sig_overTemperature();
        }
    }
    else
    {
        if(mRfDeviceFactors->rfTemperatureMeasuredA < 1000)
        {
            mRfDeviceFactors->rfTemperaturePrevious = mRfDeviceFactors->rfTemperatureMeasuredA;
        }
        mRfDeviceFactors->tmTooHighTempeatureFlag = false;
    }
#endif
    if(outputMode->getMode() == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
    {
        if(mRfDeviceFactors->rfTemperatureMeasuredA/10 > 129 && mRfDeviceFactors->rfTemperaturePreviousA > 126) //mRfDeviceFactors->rfTemperatureMeasuredB > 1290 && mRfDeviceFactors->rfTemperaturePrevious > 1260
        {
            mRfDeviceFactors->tmTooHighTempeatureFlag = true;

            if(mRfDeviceFactors->tmTooHighTemperatureDelayTime > 2)
            {
                qDebug("Temperature Over");
                RFStop();
                emit sig_overTemperature();
            }
        }
        else
        {
            if(mRfDeviceFactors->rfTemperatureMeasuredA/10 <= 130)
            {
                mRfDeviceFactors->rfTemperaturePreviousA = mRfDeviceFactors->rfTemperatureMeasuredA/10;
            }
            mRfDeviceFactors->tmTooHighTempeatureFlag = false;
           // mRfDeviceFactors->tmTooHighTemperatureDelayTime = 0;
        }
    }
    else
    {
        if(mRfDeviceFactors->rfTemperatureMeasuredB/10 > 129 && mRfDeviceFactors->rfTemperaturePreviousB > 126) //mRfDeviceFactors->rfTemperatureMeasuredB > 1290 && mRfDeviceFactors->rfTemperaturePrevious > 1260
        {
            mRfDeviceFactors->tmTooHighTempeatureFlag = true;

            if(mRfDeviceFactors->tmTooHighTemperatureDelayTime > 2)
            {
                qDebug("Temperature Over");
                RFStop();
                emit sig_overTemperature();
            }
        }
        else
        {
            if(mRfDeviceFactors->rfTemperatureMeasuredB/10 <= 130)
            {
                mRfDeviceFactors->rfTemperaturePreviousB = mRfDeviceFactors->rfTemperatureMeasuredB/10;
            }
            mRfDeviceFactors->tmTooHighTempeatureFlag = false;
           // mRfDeviceFactors->tmTooHighTemperatureDelayTime = 0;
        }
    }

}

void OutputThread::CheckSafetyState()
{

}

//void OutputThread::timerEvent(QTimerEvent *event)
//{
//    sysParm->tmElapsedTime = rfTimer.elapsed() / 1000;
//}

