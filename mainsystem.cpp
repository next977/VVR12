#include "ui_mainsystem.h"
#include "mainsystem.h"
#include "gpiotools.h"
#include "gpiobutton.h"
#include "outputthread.h"
#include "mrfa_rf_table.h"
#include "QGauge/qgauge.h"
#include <cstring>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <QFontDatabase>
#include <QDebug>
#include <QFont>
namespace global //Setting value
{             // Venistar, Continuance , Tepmperature
    uint16 TimeM[3] = {0,1,0};
    uint16 TimeS[3] = {25,0,25};
    uint16 Watt[3] = {30,100,30};
    uint16 Temp[3] = {0,0,120};
    uint16 state[8]; // TimeMUp,TimeMDown,TimeSUp,TimeSDown,WattUp,WattDown,TempUp,TempDown
    uint16 Push_state[8];
    uint16 state_en[11];
    uint16 Start;
    uint16 qgauge_visible=true;
    uint16 qgauge_enable=true;
    uint16 System_Initializing_visible=false;
    uint16 Click_state[3];
    bool cal;
    QString Display;    //connect(Setting_Screen,SIGNAL(sig_calibration()), this, SLOT(doRun()));
}

mainsystem::mainsystem(QWidget *parent) : QWidget(parent), ui(new Ui::mainsystem)
{
    ui->setupUi(this);
#ifndef _ONPC_
    gpioTools::gpio_init();

    gpioTools::uart_init();
#endif
    gpioTools::spi_init();

    sysParm = new SysParm();
    mRfDeviceFactors = sysParm->mRfDeviceFactors;

    pGButton = new GPIOButton();
    outputThread = new OutputThread(this, sysParm);
    //outputThread->start();
    EnableTimer = new QTimer(this);
    DisplayTimer = new QTimer(this);
    UpTimer = new QTimer(this);
    DownTimer = new QTimer(this);
    Setting_Screen= new setting(this, sysParm);
    Error_Screen = new sysmessage(this);
    Initializing_Screen= new initializing(this, sysParm);
    Audio_Output = new AudioOutput(this);
    DisplayTimer->setInterval(30);
    global::qgauge_visible=false; 
    Symbol_image_fix();
    setWindowFlags(Qt::FramelessWindowHint); // frame
    Setting_Screen->hide();
    Initializing_Screen->hide();
    //Initializing_Screen->close();
    global::qgauge_visible=true;
    slot_Show();
    DisplayTimer->start();
    Modechange();
    installFilter();
    ui->gaugeWidget_2->setForeground(Qt::yellow);
    ui->gaugeWidget_2->setThreshold(100);
    ui->gaugeWidget_2->setMinValue(0);
    ui->gaugeWidget_2->setMaxValue(130);
    //slot_Hide();
}


mainsystem::~mainsystem()
{
    if(outputThread) delete outputThread;
    if(sysParm) delete sysParm;
    if(pGButton) delete pGButton;

#ifndef _ONPC_
    gpioTools::uart_close();
#endif
    gpioTools::spiAllclose();
    delete ui;
}

void mainsystem::doRun()
{
    bool isCalMode = true;
    if(!isCalMode)
    {

    }
    else
    {
        SystemInitialize();
    }
}


mainsystem::State_en mainsystem::getState() const
{
    return _state;
}

void mainsystem::setState(const State_en state)
{
    if (_state != state) {
        _state = state;

        switch(state)
        {
            case STATE_SYSTEM_START:
                break;

            case STATE_SYSTEM_INITIALIZING:
                break;

            case STATE_SYSTEM_CALIBRATION:
                break;

            case STATE_POWER_READYTOACTIVE:
                break;

            case STATE_ERROR_OVER_TEMPERATURE:
                break;

            case STATE_ERROR_OVER_IMPEDANCE:
                break;

            case STATE_SYSTEM_FAIL:
                break;



            default:
                break;
       }
    }
#ifdef _ONPC_
    cout << "state=" << state << endl;
#endif
}

void mainsystem::RFParameter(bool isRead)
{
    OutputMode::RF_Output_Mode_en currentMode = outputThread->GetOutputMode();

    if(!isRead)
    {

        sysParm->tmRFPowerLevelPerModes[currentMode] = mRfDeviceFactors->rfPowerLevel;
        sysParm->tmRFCogluationTimePerModes[currentMode] = sysParm->tmRFCogluationTime;
        sysParm->tmRFCogluationTimeMPerModes[currentMode] = sysParm->tmRFCogluationTimeM; // Min add
        sysParm->tmRFCgluationTimesPerModes[currentMode] = sysParm->tmElapsedTime;
        switch(currentMode)
        {
            case OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE:
                sysParm->tmRFPowerLevelExtPerModes[currentMode] = mRfDeviceFactors->rfStepPowerLevel;
                break;

            case OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE:
                sysParm->tmRFPowerLevelExtPerModes[currentMode] = mRfDeviceFactors->rfTempLevel;
                break;
            case OutputMode::RF_DEVICE_OP_MODE_VENISTAR:
                sysParm->tmRFPowerLevelExtPerModes[currentMode] = mRfDeviceFactors->rfStepPowerLevel;
                break;

            default:
                break;
        }
    }
    else
    {
        mRfDeviceFactors->rfPowerLevel = sysParm->tmRFPowerLevelPerModes[currentMode];
        sysParm->tmRFCogluationTime = sysParm->tmRFCogluationTimePerModes[currentMode];
        sysParm->tmRFCogluationTimeM = sysParm->tmRFCogluationTimeMPerModes[currentMode]; // Min add
        sysParm->tmElapsedTime = sysParm->tmRFCgluationTimesPerModes[currentMode] ;
        switch(currentMode)
        {
            case OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE:
                mRfDeviceFactors->rfStepPowerLevel = sysParm->tmRFPowerLevelExtPerModes[currentMode];
                break;

            case OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE:
                mRfDeviceFactors->rfTempLevel = sysParm->tmRFPowerLevelExtPerModes[currentMode];
                break;
            case OutputMode::RF_DEVICE_OP_MODE_VENISTAR:
                mRfDeviceFactors->rfStepPowerLevel = sysParm->tmRFPowerLevelExtPerModes[currentMode];
                break;
            default:
                break;
        }
        global::Watt[currentMode] = mRfDeviceFactors->rfPowerLevel;
        global::Temp[currentMode] = mRfDeviceFactors->rfTempLevel;
        global::TimeS[currentMode] = sysParm->tmRFCogluationTime;
        global::TimeM[currentMode] = sysParm->tmRFCogluationTimeM;
    }
}

void mainsystem::SystemInitialize()
{
    uint16 voltage_test, current_test, temperature_test, power_test, impedance_test, test_reulst;
    mRfDeviceFactors->System_result = true;
    //sysParm->tmElapsedTimeM = 0;
    setState(STATE_SYSTEM_INITIALIZING);

    voltage_test = mRfDeviceFactors->tmReadADCVoltage;
    current_test = mRfDeviceFactors->tmReadADCCurrent;
    temperature_test = mRfDeviceFactors->tmReadADCTemperatureA;

    if((voltage_test<TEST_ADC_VOLTAGE)&&(voltage_test>10))
    {

    }
    else
    {
        mRfDeviceFactors->System_result = false;
    }
    if((current_test<TEST_ADC_CURRENT)&&(current_test>10))
    {

    }
    else
    {

        mRfDeviceFactors->System_result = false;
    }
    if((TEST_ADC_TEMPERATURE-200<temperature_test)&&(temperature_test<TEST_ADC_TEMPERATURE+200))
    {

    }
    else
    {

        mRfDeviceFactors->System_result = false;
    }

#if 1
    //gpioTools::DACPortOutput(mRfDeviceFactors->tmDACValueToSetLast);

    //sysparam.tmDeviceControlMode = RF_DEVICE_IMPCTRL_MODE_CHECK;

   // EnableAmplifierPowerSource(0);
   // EnableAmplifierPowerOutput(0,RFOUTPUT_DIR_DUMMY);

    gpioTools::EnableAmplifierPowerOutput();
    gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_DUMMY);
    //gpioTools::SetElectrodeType(gpioTools::ELECTRODE_MONOPOLAR);

    mRfDeviceFactors[0].tmDACValueToSetLast = sysParm->GetWattToDACValuefor1Watt();
   // mRfDeviceFactors[RF_DEVICE_ID0].tmAmplifierActive = true;
    sysParm->isDoTestforCal = true;
   // mdelay(3000);
   // OSTimeDly(3000);

    power_test = mRfDeviceFactors[0].rfPowerMeasured/100;
    if( mRfDeviceFactors[0].rfImpedanceMeasured >= 9990 )
    {
        mRfDeviceFactors[0].rfImpedanceMeasured = 999;
    }

    impedance_test= mRfDeviceFactors[0].rfImpedanceMeasured;
   // mdelay(1000);
   // OSTimeDly(1000);

    //gpioTools::SetElectrodeType(gpioTools::ELECTRODE_NONE);
    gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_DUMMY);
    gpioTools::DisableAmplifierPowerOutput();

    //DisableAmplifierPowerSource(0);
    //DisableAmplifierPowerOutput(0,RFOUTPUT_DIR_DUMMY);
    //DisableAmplifierPowerLED(0);
    mRfDeviceFactors[0].tmDACValueToSetLast = 0;
   // mRfDeviceFactors[RF_DEVICE_ID0].tmAmplifierActive = false;
    sysParm->isDoTestforCal = false;
  //  sysparam.tmDeviceControlMode = RF_DEVICE_IMPCTRL_MODE_NORMAL;
#endif

    if((TEST_POWER/2-3<=power_test)&&(power_test<=TEST_POWER/2+5))
    {

    }
    else
    {
        mRfDeviceFactors->System_result = false;
    }
    if((TEST_DUMMY_LOAD-30<impedance_test)&&(impedance_test<TEST_DUMMY_LOAD+30))
    {

    }
    else
    {

        mRfDeviceFactors->System_result = false;
    }
    mRfDeviceFactors->System_result = true;
#ifndef __FEATURE_VVR12_BOARD__
    slot_Hide();
    Initializing_Screen->show();
    global::System_Initializing_visible=true;
#endif
    if(mRfDeviceFactors->System_result)
    {

        FinishInitialize();
    }
    else
    {
        setState(STATE_SYSTEM_FAIL);
    }
}

void mainsystem::FinishInitialize()
{

    diplayTimer.start(50);
    //EnableTimer->start();
    connect(&diplayTimer, SIGNAL(timeout()), this, SLOT(slot_update()));

    ////////////////////////////////////////////////////////////////////////////
    connect(EnableTimer, SIGNAL(timeout()), this, SLOT(slot_EnableTime())); //Timer
    connect(DisplayTimer, SIGNAL(timeout()), this, SLOT(slot_Display())); //Timer
    ////////////////////////////////////////////////////////////////////////////
    connect(ui->CM_radio_Three,SIGNAL(clicked()), this, SLOT(slot_CM_Mode_Clicked()));
    connect(ui->CM_radio_Seven,SIGNAL(clicked()), this, SLOT(slot_CM_Mode_Clicked()));
   // connect(ui->System_Message,SIGNAL(clicked()), this, SLOT(SysMessage_clicked()));
    connect(pGButton, SIGNAL(sig_btnReleased(int)), this, SLOT(slot_btnReleased(int)));
    connect(pGButton, SIGNAL(sig_btnLongPressed(int)), this, SLOT(slot_btnLongPressed(int)));
    connect(this,SIGNAL(sig_Wattgauge(int)),ui->gaugeWidget,SLOT(setValue(int)));
    connect(this,SIGNAL(sig_Tempgauge(int)),ui->gaugeWidget_2,SLOT(setValue1(int)));
    connect(this, SIGNAL(sig_btnReleased(int)), this, SLOT(slot_btnReleased(int)));
    connect(this, SIGNAL(sig_btnDirReleased(int)), this, SLOT(slot_btnDirReleased(int)));
    connect(this, SIGNAL(sig_btnLongPressed(int)), this, SLOT(slot_btnLongPressed(int)));
    connect(this, SIGNAL(sig_Sysmessage(mainsystem::State_en)), this, SLOT(slot_SysMessaged(mainsystem::State_en)));
   // connect(this, SIGNAL(sig_audiostate(AudioOutput::eAUDIOS)), Audio_Output, SLOT(slot_playSound(AudioOutput::eAUDIOS)));
   // connect(this, SIGNAL(sig_audiostate()), Audio_Output, SLOT(playSound_state()));
    connect(Setting_Screen, SIGNAL(sig_btnDirReleased(int)), this, SLOT(slot_btnDirReleased(int)));
    connect(Setting_Screen, SIGNAL(sig_btnReleased(int)), this, SLOT(slot_btnReleased(int)));
    connect(Setting_Screen, SIGNAL(sig_setting()), this, SLOT(slot_Setting_Enable()));
    connect(Initializing_Screen, SIGNAL(sig_init()), this, SLOT(slot_init_screen()));
    connect(outputThread, SIGNAL(sig_modeChange(OutputMode::RF_Output_Mode_en)), this, SLOT(slot_modeChanged(OutputMode::RF_Output_Mode_en)));
    connect(outputThread, SIGNAL(sig_overTemperature()), this, SLOT(slot_overTemperatureWarning()));
#ifdef _NOT_YET_
    connect(outputThread, SIGNAL(sig_shortCircuit()), this, SLOT(slot_shortCircuitWarning()));
#endif
    connect(outputThread, SIGNAL(sig_lowImpedance()), this, SLOT(slot_lowImpedanceWarning()));
    connect(outputThread, SIGNAL(sig_overImpedance()), this, SLOT(slot_overImpedanceWarning()));

    connect(outputThread, SIGNAL(sig_rfStart()), this, SLOT(slot_rfStarted()));
    connect(outputThread, SIGNAL(sig_rfStop()), this, SLOT(slot_rfStopped()));
#ifdef Continu_Mode_Start
   outputThread->SetOutputMode(OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE);
#else
   outputThread->SetOutputMode(OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE);
   #ifndef __FEATURE_VVR12_BOARD__
   outputThread->SetOutputCMMode(OutputMode::RF_DEVICE_OP_MODE_3CM);
#endif
#endif
   Temperature_CM_Mode = RF_DEVICE_OP_MODE_3CM;
   Venistar_CM_Mode = RF_DEVICE_OP_MODE_3CM;
   setState(STATE_POWER_READYTOACTIVE);
   Hide_Show_Display();
   //Audio_Output->stopSound();
}

void mainsystem::slot_update()
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    /*
     *  Temperature
     */
    if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
    {

        if(mRfDeviceFactors->tmReadADCTemperatureA <= sysParm->GetLastADTemperatureA()) //connect probe
        {
            emit sig_Tempgauge((int)mRfDeviceFactors->rfTemperatureMeasuredA/10);
        }
        else
        {
            emit sig_Tempgauge((int)mRfDeviceFactors->rfTemperatureMeasuredA/10);
        }

   }
    else
    {
        if(mRfDeviceFactors->tmReadADCTemperatureB <= sysParm->GetLastADTemperatureB()) //conect probe
        {
            emit sig_Tempgauge((int)mRfDeviceFactors->rfTemperatureMeasuredB/10);
        }
        else
        {
            emit sig_Tempgauge((int)mRfDeviceFactors->rfTemperatureMeasuredB/10);
        }
    }
    emit sig_Wattgauge((int)mRfDeviceFactors->rfPowerMeasured/100);
    if(getState() == STATE_POWER_ACTIVE)
    {
        /*
         * Watt
         */
        /*
        if( mRfDeviceFactors->tmMRFAState == OutputMode::RF_STATE_STOP )
        {
            DisplayRealWatt(0);
            //emit sig_Wattgauge(0);
        }
        else
        {
            DisplayRealWatt(mRfDeviceFactors->rfPowerMeasured/100);
           // emit sig_Wattgauge((int)mRfDeviceFactors->rfPowerMeasured/100);
        }
        */
        /*
         * Time
         */

//display ss
            sysParm->tmElapsedTime = static_cast<uint32>(elapsedTime.elapsed()/1000);
#if 0
        if(sysParm->tmElapsedTime != elapsedTime.elapsed()/1000)
        {

            int i;


#ifndef __FEATURE_VVR12_BOARD__
           // sysParm->tmElapsedTime = static_cast<uint32>(elapsedTime.elapsed()/1000);
           // strTime.sprintf("%3d",  sysParm->tmElapsedTime);
#endif





            if(sysParm->tmElapsedTimeM < sysParm->tmRFCogluationTimeM)
            {
                if(sysParm->tmElapsedTime < 59)
                {
                    sysParm->tmElapsedTime = static_cast<uint32>(elapsedTime.elapsed()/1000);
                    strTime.sprintf("%3d",  sysParm->tmElapsedTime);
                }	//or 60
               else
               {


                    elapsedTime.restart();
                    sysParm->tmElapsedTime = static_cast<uint32>(elapsedTime.elapsed()/1000);
                    qDebug("%d",sysParm->tmElapsedTimeM);
                    sysParm->tmElapsedTimeM ++;
                    qDebug("%d",sysParm->tmElapsedTimeM);
                }
            }
            else
            {
                sysParm->tmElapsedTime = static_cast<uint32>(elapsedTime.elapsed()/1000);
                strTime.sprintf("%3d",  sysParm->tmElapsedTime);
#ifndef __FEATURE_VVR12_BOARD__
                //if(TimeS2[1] < (global1::TimeS[1])) TimeS2[1]++;
                //else Start = false;
#endif
            }
            if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE) //continu mode
            {
                Display_TimeM[0]=sysParm->tmElapsedTimeM;
                Display_TimeS[0]=sysParm->tmElapsedTime;
            }
            else           //temp mode
            {

                Display_TimeM[1]=sysParm->tmElapsedTimeM;
                Display_TimeS[1]=sysParm->tmElapsedTime;
            }

        }
#endif

#ifndef _ONPC_
        if(uartTime != sysParm->tmElapsedTime)
        {
            gpioTools::uart_printf("Data,%d,%d,%d,%d,%d,0,0,0,%d,0,0,0,0,%d,0,0\n\r",
            sysParm->tmElapsedTime,
            mRfDeviceFactors->rfPowerMeasured/100,
            mRfDeviceFactors->rfCurrentMeasured,
            mRfDeviceFactors->rfImpedanceMeasured,	//2009.2.13(modified)
            mRfDeviceFactors->rfTemperatureMeasuredB/10,	//2008.2.5(modified)
            mRfDeviceFactors->rfTemperatureMeasuredA/10,
            mRfDeviceFactors->tmTotalJ/10);
        }
#else
        if(uartTime != sysParm->tmElapsedTime)
        {
            qDebug("Data,%d,%d,%d,%d,%d,0,0,0,%d,0,0,0,0,%d,0,0\n",
            sysParm->tmElapsedTime,
            mRfDeviceFactors->rfPowerMeasured/100,
            mRfDeviceFactors->rfCurrentMeasured,
            mRfDeviceFactors->rfImpedanceMeasured,	//2009.2.13(modified)
            mRfDeviceFactors->rfTemperatureMeasuredA/10,	//2008.2.5(modified)
            mRfDeviceFactors->rfTemperatureMeasuredB/10,
            mRfDeviceFactors->tmTotalJ/10);
        }
#endif

        uartTime = static_cast<int32>(sysParm->tmElapsedTime);
    }

}

void mainsystem::slot_modeChanged(OutputMode::RF_Output_Mode_en mode)
{
    RFParameter(true);
    if(mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
    {
        //strMode = "TEMPERATURE mode\nTemperature ";
    }
    else
    {
        //strMode = "CONTINUANCE mode\nWatt ";
    }
    //DisplayActiveReady(0);
}

void mainsystem::slot_rfStarted()
{

    elapsedTime.restart();
    sysParm->tmElapsedTime= 0;
    uartTime = -1;
    setState(STATE_POWER_ACTIVE);
    Hide_Show_Display();
#ifndef _ONPC_
    //gpioTools::uart_printf(const_cast<int8*>("[START]"));
#endif
}

void mainsystem::slot_rfStopped()
{
    setState(STATE_POWER_STOP);
    setState(STATE_POWER_READYTOACTIVE);
    Hide_Show_Display();
#ifndef _ONPC_
    //gpioTools::uart_printf(const_cast<int8*>("[STOP]"));
#endif
    //DisplayRealWatt(0);
}

void mainsystem::slot_btnReleased(int keyEvent)
{
    switch(keyEvent)
    {
        case BTNS::BTN_ELECTRODE_RFSTARTSTOP :
            if(getState() == STATE_POWER_ACTIVE)
            {
                qDebug("RF_Stop");
                outputThread->RFStop();
                //Audio_Output->playSound(AudioOutput::SND_RFSTOP);
                Audio_Output->stopSound();
            }
            break;

        case BTNS::BTN_RFSTARTSTOP :
           if(getState() == STATE_POWER_ACTIVE)
           {
               qDebug("RF_Stop");
               outputThread->RFStop();
               //Audio_Output->playSound(AudioOutput::SND_RFSTOP);
               Audio_Output->stopSound();
           }
           break;

        case BTNS::BTN_MODE:
            if(!outputThread->GetRFPowerActiveState())
            {
                if((getState() == STATE_POWER_READYTOACTIVE)||(getState() == STATE_SYSTEM_MODE_SETTING))
                {
                    RFParameter(false);
                    outputThread->SetOutputMode();   //event modeChanged
                    Audio_Output->playSound(AudioOutput::SND_RFSTART);
                   // emit sig_audiostate(AudioOutput::SND_RFSTART);
                    emit sig_audiostate();

                }
            }
#ifndef __FEATURE_VVR12_BOARD__
            /*
            if((getState() == STATE_POWER_READYTOACTIVE)) //||(getState() == STATE_SYSTEM_MODE_SETTING)
            {

                outputThread->SetOutputMode();   //event modeChanged

            }
            */
#endif
            break;


        case BTNS::BTN_ENCORDER_SW:
            if(getState() == STATE_SYSTEM_MODE_SETTING)
            {
                sysParm->modeSettingParam++;
                gpioTools::StopSound();
#ifndef __FEATURE_VVR12_BOARD__
                //gpioTools::StartSound(WAVE_keytmp);
#endif
                switch(sysParm->modeSettingParam)
                {
                    case 0:		//temperature level setting
                    case 1:
                    case 2:
                        //go out from menu page
                        //DisplayActiveReady(0);
                        setState(STATE_POWER_READYTOACTIVE);
                        diplayTimer.start(50);
                        break;
                }
            }
            else if(getState() == STATE_POWER_READYTOACTIVE)
            {
                StartMenuSetting();

            }
            break;

        default:
            break;
    }
}

void mainsystem::slot_btnDirReleased(int keyEvent)
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
#ifndef __FEATURE_VVR12_BOARD__
    /*
    if(getState() == STATE_POWER_ACTIVE)
    {
#ifdef __FEATURE_RF_ACTIVE_ADJUSTABLE__
        if(keyEvent == BTNS::BTN_DR_RIGHT)
        {
            if( mRfDeviceFactors->rfPowerLevel < 100)
            {
                if(mRfDeviceFactors->rfPowerLevel<20)
                {
                    if(mRfDeviceFactors->rfPowerLevel == 0)
                        mRfDeviceFactors->rfPowerLevel = 5;
                    else
                        mRfDeviceFactors->rfPowerLevel += 1;
                }
                else
                {
                    mRfDeviceFactors->rfPowerLevel += 5;
                }

                if(mRfDeviceFactors->rfPowerLevel > 100)
                {
                    mRfDeviceFactors->rfPowerLevel = 100;
                }

                mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                //StopSound();
                //StartSound(WAVE_keylock);
                DisplaySetParameter(1);
            }
        }
        else if(keyEvent == BTNS::BTN_DR_LEFT)
        {
            if( mRfDeviceFactors->rfPowerLevel > 0)	//5)
            {
                if(mRfDeviceFactors->rfPowerLevel <= 20)
                {
                    if(mRfDeviceFactors->rfPowerLevel <= 5)
                        mRfDeviceFactors->rfPowerLevel = 0;
                    else
                        mRfDeviceFactors->rfPowerLevel -= 1;
                }
                else
                {
                    if(mRfDeviceFactors->rfPowerLevel < 5)
                        mRfDeviceFactors->rfPowerLevel = 0;
                    else
                        mRfDeviceFactors->rfPowerLevel -= 5;
                }
                if(mRfDeviceFactors->rfPowerLevel == 0)
                {
                    mRfDeviceFactors->rfPowerLevel = 0;
                    //mRfDeviceFactors->rfPowerLevelSave = 0;
                    mRfDeviceFactors->tmDACValueToSetLast=0;
                }
                mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                //StopSound();
                //StartSound(WAVE_keylock);
                DisplaySetParameter(1);
            }
        }
#endif
    }
    */
#endif
    if((getState() == STATE_POWER_ACTIVE)||(getState() == STATE_SYSTEM_MODE_SETTING))
    {
        if(keyEvent == BTNS::BTN_DR_RIGHT)
        {

            if(global::Push_state[1]==true)
            {
                if( mRfDeviceFactors->rfTempLevel < 120) //100
                {
#ifndef __FEATURE_VVR12_BOARD__
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    if(mRfDeviceFactors->rfTempLevel%10==0)
                    {
                        i[1]++;
                    }
                    if(i[1]>0)
                        mRfDeviceFactors->rfTempLevel += 5;
                    else
                    {
                        mRfDeviceFactors->rfTempLevel += 1;
                    }

                    DisplaySetParameter(0); //sysParm->modeSettingParam
                }
                global::Push_state[1]=false;
            }
            else if(global::state[6]==true)// sysParm->modeSettingParam == 0
            {
                i[1] = 0;
                if( mRfDeviceFactors->rfTempLevel < 120) //100
                {
#ifndef __FEATURE_VVR12_BOARD__
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    if(Step[0]==true)
                        mRfDeviceFactors->rfTempLevel += 5;
                    else
                        mRfDeviceFactors->rfTempLevel += 1;
                    if(mRfDeviceFactors->rfTempLevel > 120)
                        mRfDeviceFactors->rfTempLevel = 120;
                    DisplaySetParameter(0); //sysParm->modeSettingParam

                }
                global::state[6]=false;
                Step[0] = false;

            }
            else if(global::Push_state[0]==true) // sysParm->modeSettingParam == 1
            {
                if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
                {
                    if( mRfDeviceFactors->rfPowerLevel < 100)	//150)
                    {

                        mRfDeviceFactors->rfPowerLevel += 50;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }
                else
                {
                    if( mRfDeviceFactors->rfPowerLevel < 100)	//150)
                    {
                        if(mRfDeviceFactors->rfPowerLevel%10==0)
                        {
                            i[0]++;
                        }
                        if(i[0]>0)
                            mRfDeviceFactors->rfPowerLevel += 5;
                        else
                            mRfDeviceFactors->rfPowerLevel += 1;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }

                global::Push_state[0]=false;
            }
            else if(global::state[4]==true) // sysParm->modeSettingParam == 1
            {
                i[0] = 0;
                if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
                {
                    if( mRfDeviceFactors->rfPowerLevel < 100)	//150)
                    {

                        mRfDeviceFactors->rfPowerLevel += 50;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }
                else
                {
                    if( mRfDeviceFactors->rfPowerLevel < 100)	//150)
                    {
    #ifndef __FEATURE_VVR12_BOARD__
                        /*
                        if(global::mode==RF_MODE_TEMPERATURE)
                        {

                            if(mRfDeviceFactors->rfPowerLevel<20)
                            {
                                if(mRfDeviceFactors->rfPowerLevel == 0)
                                    mRfDeviceFactors->rfPowerLevel = 5;
                                else
                                    mRfDeviceFactors->rfPowerLevel += 1;
                            }
                            else
                            {
                                mRfDeviceFactors->rfPowerLevel += 5;
                            }
                            mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;

                        }
                        else
                        {
                            mRfDeviceFactors->rfPowerLevel += 5;
                            mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        }
                        */
                        //mRfDeviceFactors->tmDACValueToSet = MRFA_GetWattToDACValue(sysparam.tmActiveChannelIndex, mRfDeviceFactors[sysparam.tmActiveChannelIndex].rfPowerLevel);

                        //StopSound();
                        //StartSound(WAVE_keylock);
    #endif
                        if(Step[0]==true)
                            mRfDeviceFactors->rfPowerLevel += 5;
                        else
                            mRfDeviceFactors->rfPowerLevel += 1;
                        if(mRfDeviceFactors->rfPowerLevel>100)
                            mRfDeviceFactors->rfPowerLevel = 100;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }

                global::state[4]=false;
                Step[0]= false;
            }
            else if(global::Push_state[4]==true) // sysParm->modeSettingParam == 2
            {
                if( sysParm->tmRFCogluationTime < 59 ) //900
                {
                    if(sysParm->tmRFCogluationTime%10==0)
                    {
                        i[4]++;
                    }
                    if(i[4]>0)
                    {
                        if(sysParm->tmRFCogluationTime<55)
                            sysParm->tmRFCogluationTime += 5;
                        else
                            sysParm->tmRFCogluationTime += 1;
                    }
                    else
                        sysParm->tmRFCogluationTime += 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(2); //sysParm->modeSettingParam
                }
                global::Push_state[4]=false;
            }
            else if(global::state[2]==true) // sysParm->modeSettingParam == 2
            {
                i[4] = 0;
                if( sysParm->tmRFCogluationTime < 59 ) //900
                {
                    sysParm->tmRFCogluationTime += 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(2); //sysParm->modeSettingParam
                }
                global::state[2]=false;
            }
            else if(global::Push_state[6]==true) // sysParm->modeSettingParam == 2
            {
                if( sysParm->tmRFCogluationTimeM < 59 ) //12
                {
                    if(sysParm->tmRFCogluationTimeM%10==0)
                    {
                        i[6]++;
                    }
                    if(i[6]>0)
                    {
                        if(sysParm->tmRFCogluationTimeM<55)
                            sysParm->tmRFCogluationTimeM += 5;
                        else
                            sysParm->tmRFCogluationTimeM += 1;
                    }
                    else
                        sysParm->tmRFCogluationTimeM += 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(3); //sysParm->modeSettingParam
                }
                global::Push_state[6]=false;
            }
            else if(global::state[0]==true) // sysParm->modeSettingParam == 2
            {
                 i[6] = 0;
                if( sysParm->tmRFCogluationTimeM < 59 ) //12
                {
                    sysParm->tmRFCogluationTimeM += 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(3); //sysParm->modeSettingParam
                }
                global::state[0]=false;
            }


        }
        else if( keyEvent == BTNS::BTN_DR_LEFT)
        {
            if(global::Push_state[3]==true)
            {
                if( mRfDeviceFactors->rfTempLevel > 60) //100
                {
#ifndef __FEATURE_VVR12_BOARD__
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    if(mRfDeviceFactors->rfTempLevel%10==0)
                    {
                        i[3]++;
                    }
                    if(i[3]>0)
                        mRfDeviceFactors->rfTempLevel -= 5;
                    else
                        mRfDeviceFactors->rfTempLevel -= 1;
                    DisplaySetParameter(0); //sysParm->modeSettingParam
                }
                global::Push_state[3]=false;
            }
            else if(global::state[7]==true) //sysParm->modeSettingParam == 0
            {
                i[3] = 0;
                if( mRfDeviceFactors->rfTempLevel > 60)
                {
                    if(Step[1]==true)
                        mRfDeviceFactors->rfTempLevel -= 5;
                    else
                        mRfDeviceFactors->rfTempLevel -= 1;
                    if(mRfDeviceFactors->rfTempLevel <60)
                        mRfDeviceFactors->rfTempLevel = 60;
#ifndef __FEATURE_VVR12_BOARD__
                    //StopSound();
                    //StartSound(WAVE_keyun);
#endif
                    DisplaySetParameter(0);
                }
                global::state[7]=false;
                Step[1] = false;
            }
            else if(global::Push_state[2]==true) // sysParm->modeSettingParam == 1
            {
                if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
                {
                    if( mRfDeviceFactors->rfPowerLevel > 0)	//150)
                    {

                        mRfDeviceFactors->rfPowerLevel -= 50;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }
                else
                {
                    if( mRfDeviceFactors->rfPowerLevel > 0)	//150)
                    {
                        if(mRfDeviceFactors->rfPowerLevel%10==0)
                        {
                            i[2]++;
                        }
                        if(i[2]>0)
                            mRfDeviceFactors->rfPowerLevel -= 5;
                        else
                            mRfDeviceFactors->rfPowerLevel -= 1;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }

                global::Push_state[2]=false;
            }
            else if(global::state[5]==true) //sysParm->modeSettingParam == 1
            {
                i[2] = 0;
                if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
                {
                    if( mRfDeviceFactors->rfPowerLevel > 0)	//150)
                    {

                        mRfDeviceFactors->rfPowerLevel -= 50;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1); //sysParm->modeSettingParam

                    }
                }
                else
                {
                    if( mRfDeviceFactors->rfPowerLevel > 0)	//5)
                    {
    #ifndef __FEATURE_VVR12_BOARD__
                        /*
                        if(global::mode == RF_MODE_TEMPERATURE)
                        {
                            if(mRfDeviceFactors->rfPowerLevel<=20)
                            {
                                if(mRfDeviceFactors->rfPowerLevel <= 5)
                                    mRfDeviceFactors->rfPowerLevel = 0;
                                else
                                    mRfDeviceFactors->rfPowerLevel -= 1;
                            }
                            else
                            {
                                mRfDeviceFactors->rfPowerLevel -= 5;
                            }

                            mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        }
                        else
                        {
                                mRfDeviceFactors->rfPowerLevel -= 5;
                                mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        }
                        */
                        //StopSound();
                        //StartSound(WAVE_keyun);
    #endif
                        if(Step[1]==true)
                            mRfDeviceFactors->rfPowerLevel -= 5;
                        else
                            mRfDeviceFactors->rfPowerLevel -= 1;
                        if(mRfDeviceFactors->rfPowerLevel < 0)
                            mRfDeviceFactors->rfPowerLevel = 0;
                        mRfDeviceFactors->rfPowerLevel_Init = mRfDeviceFactors->rfPowerLevel;
                        DisplaySetParameter(1);
                    }
                }

                global::state[5]=false;
                Step[1]=false;
            }
            else if(global::Push_state[5]==true) // sysParm->modeSettingParam == 2
            {
                if( sysParm->tmRFCogluationTime > 0 ) //900
                {
                    if(sysParm->tmRFCogluationTime%10==0)
                    {
                        i[5]++;
                    }
                    if(i[5]>0)
                    {
                        if(sysParm->tmRFCogluationTime > 5)
                            sysParm->tmRFCogluationTime -= 5;
                        else
                            sysParm->tmRFCogluationTime -= 1;
                    }
                    else
                        sysParm->tmRFCogluationTime -= 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(2); //sysParm->modeSettingParam
                }
                global::Push_state[5]=false;
            }
            else if(global::state[3]==true) // sysParm->modeSettingParam == 2
            {
                i[5]=0;
                if( sysParm->tmRFCogluationTime > 0 ) //900
                {
                    sysParm->tmRFCogluationTime -= 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(2); //sysParm->modeSettingParam
                }
                global::state[3]=false;
            }
            else if(global::Push_state[7]==true) // sysParm->modeSettingParam == 2
            {
                if( sysParm->tmRFCogluationTimeM > 0 ) //900
                {
                    if(sysParm->tmRFCogluationTimeM%10==0)
                    {
                        i[7]++;
                    }
                    if(i[7]>0)
                    {
                        if(sysParm->tmRFCogluationTimeM > 5)
                            sysParm->tmRFCogluationTimeM -= 5;
                        else
                            sysParm->tmRFCogluationTimeM -= 1;
                    }
                    else
                        sysParm->tmRFCogluationTimeM -= 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(3); //sysParm->modeSettingParam
                }
                global::Push_state[7]=false;
            }
            else if(global::state[1]==true) // sysParm->modeSettingParam == 2
            {
                i[7] = 0;
                if( sysParm->tmRFCogluationTimeM > 0 ) //900
                {
                    sysParm->tmRFCogluationTimeM -= 1;
#ifndef __FEATURE_VVR12_BOARD__
                    //if(global1::TimeS[0]<59) global1::TimeS[0] ++;
                    //StopSound();
                    //StartSound(WAVE_keylock);
#endif
                    DisplaySetParameter(3); //sysParm->modeSettingParam
                }
                global::state[1]=false;
            }
        }
    }
}
void mainsystem::slot_btnLongPressed(int keyEvent)
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    if((keyEvent == BTNS::BTN_ELECTRODE_RFSTARTSTOP)&&(Mode != OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE ))
    {
        qDebug("Electrode_start");
        if(getState() == STATE_POWER_READYTOACTIVE)
        {
            qDebug("RF_Start");
            outputThread->RFStart();
           // Audio_Output->playSound(AudioOutput::SND_RFSTART);
        }
    }
    else if(keyEvent == BTNS::BTN_RFSTARTSTOP)
    {
        if(getState() == STATE_POWER_READYTOACTIVE)
        {
            qDebug("RF_Start");
            outputThread->RFStart();
           // Audio_Output->playSound(AudioOutput::SND_RFSTART);
        }
    }
    else if(keyEvent == BTNS::BTN_OHMECHECK)
    {
    }
}
void mainsystem::StartMenuSetting()
{
    setState(STATE_SYSTEM_MODE_SETTING);
    diplayTimer.stop();

    switch(outputThread->GetOutputMode())
    {
        case OutputMode::RF_DEVICE_OP_MODE_VENISTAR:
            sysParm->modeSettingParam = 0; //
            //number
            DisplaySetParameter(sysParm->modeSettingParam);
            break;
        case OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE:
            sysParm->modeSettingParam = 0; //
            //number
            DisplaySetParameter(sysParm->modeSettingParam);
            break;
        case OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE:
            sysParm->modeSettingParam = 0; //0
            DisplaySetParameter(sysParm->modeSettingParam);
            break;
        default:
            break;
    }

}
void mainsystem::slot_overTemperatureWarning()
{
    outputThread->RFStop();

    setState(STATE_ERROR_OVER_TEMPERATURE);
    emit sig_Sysmessage(STATE_ERROR_OVER_TEMPERATURE);
#ifndef __FEATURE_VVR12_BOARD__
    //show warning message during 2sec
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED1);
    //gpioTools::OnLED(gpioTools::Leds_ENCODER_LED2);
    //OLED_FillDisplay(0x00);
    //OutputString5x7(overTempMsg, 12, 6, strlen(reinterpret_cast<const int8*>(overTempMsg)));
   // mdelay(2000);
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED2);

    //DisplayActiveReady(0);
#endif
    setState(STATE_POWER_READYTOACTIVE);
}
void mainsystem::slot_lowImpedanceWarning()
{
    outputThread->RFStop();

    setState(STATE_ERROR_LOW_IMPEDANCE);
    emit sig_Sysmessage(STATE_ERROR_LOW_IMPEDANCE);
#ifndef __FEATURE_VVR12_BOARD__
    //show warning message during 2sec
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED1);
    //gpioTools::OnLED(gpioTools::Leds_ENCODER_LED2);
    //OLED_FillDisplay(0x00);
    //OutputString5x7(overImpMsg, 12, 6, strlen(reinterpret_cast<const int8*>(overImpMsg)));
    //mdelay(2000);
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED2);

    //DisplayActiveReady(0);
#endif
    setState(STATE_POWER_READYTOACTIVE);
}

void mainsystem::slot_overImpedanceWarning()
{
    outputThread->RFStop();

    setState(STATE_ERROR_OVER_IMPEDANCE);
    emit sig_Sysmessage(STATE_ERROR_OVER_IMPEDANCE);
#ifndef __FEATURE_VVR12_BOARD__
    //show warning message during 2sec
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED1);
    //gpioTools::OnLED(gpioTools::Leds_ENCODER_LED2);
    //OLED_FillDisplay(0x00);
    //OutputString5x7(overImpMsg, 12, 6, strlen(reinterpret_cast<const int8*>(overImpMsg)));
    //mdelay(2000);
    //gpioTools::OffLED(gpioTools::Leds_ENCODER_LED2);

    //DisplayActiveReady(0);
#endif
    setState(STATE_POWER_READYTOACTIVE);
}

void mainsystem::LCD_Display(mainsystem::RF_mode mode)
{
    QString strTime;

    strTime.sprintf("%02d:%02d", sysParm->tmElapsedTime/60, sysParm->tmElapsedTime%60);
    ui->TimeLCD->display(strTime);
//    ui->DAC_LCD->display(sysParm->mRfDeviceFactors->tmDACValueToSetLast);
#ifndef __VVR12_BOARD_TEST__
    ui->V_LCD->display(sysParm->mRfDeviceFactors->tmReadADCVoltage);
    ui->T_A_LCD->display(sysParm->mRfDeviceFactors->tmReadADCTemperatureA);
    ui->T_B_LCD->display(sysParm->mRfDeviceFactors->tmReadADCTemperatureB);
    ui->I_LCD->display(sysParm->mRfDeviceFactors->tmReadADCCurrent);
    ui->R_LCD->display(sysParm->mRfDeviceFactors->rfImpedanceMeasured);
    ui->DAC_LCD->display(sysParm->mRfDeviceFactors->tmDACValueToSetLast);
#endif

}

void mainsystem::Setting_Display(mainsystem::RF_mode mode)
{
    global::Display.sprintf("%02d:%02d", global::TimeM[mode], global::TimeS[mode]);
    ui->Timeset->display(global::Display);
    ui->Tempset->display(global::Temp[mode]);
    ui->Wattset->display(global::Watt[mode]);
}

void mainsystem::slot_init_screen()
{

    // w.showMaximized();
    ui->gaugeWidget_2->setForeground(Qt::yellow);
    ui->gaugeWidget_2->setThreshold(100);
    ui->gaugeWidget_2->setMinValue(0);
    ui->gaugeWidget_2->setMaxValue(130);
    Initializing_Screen->hide();
    global::qgauge_visible=true;
    slot_Show();
    DisplayTimer->start();
    Modechange();

}

void mainsystem::DisplaySetParameter(int32 param)
{
    QString setParm;
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    switch(param)
    {
        case 0:	//Temp
            setParm.sprintf("%3d", mRfDeviceFactors->rfTempLevel);
            global::Temp[2] = mRfDeviceFactors->rfTempLevel;
            break;
        case 1:	//Watt
            setParm.sprintf("%3d", mRfDeviceFactors->rfPowerLevel);
            global::Watt[Mode] = mRfDeviceFactors->rfPowerLevel;
            break;

        case 2:	//time sec
            setParm.sprintf("%3d", sysParm->tmRFCogluationTime);
            global::TimeS[Mode] = sysParm->tmRFCogluationTime;
            break;
        case 3:	//time Min
            setParm.sprintf("%3d", sysParm->tmRFCogluationTimeM);
            global::TimeM[Mode] = sysParm->tmRFCogluationTimeM;
            break;
    }
}

void mainsystem::Modechange()
{
    OutputMode::RF_Output_Mode_en mode = outputThread->GetOutputMode();
    if(mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE) //Temperature Mode
     {
        ui->Tempset->hide();

     }
    else
    {

         ui->Tempset->show();
     }
}

void mainsystem::slot_SysMessaged(mainsystem::State_en state)  //W2: System message dialog
{
    switch(state)
    {
        case STATE_SYSTEM_START:
            break;

        case STATE_SYSTEM_INITIALIZING:
            break;

        case STATE_SYSTEM_CALIBRATION:
            break;

        case STATE_POWER_READYTOACTIVE:
            break;

        case STATE_ERROR_OVER_TEMPERATURE:
            global::state_en[8]=true;
            break;

        case STATE_ERROR_OVER_IMPEDANCE:
             global::state_en[9]=true;
            break;
        case STATE_ERROR_LOW_IMPEDANCE:
             global::state_en[10]=true;
            break;
        case STATE_SYSTEM_FAIL:
             global::state_en[0]=true;
            break;
        default:
            break;
   }
    MessageTimer.restart();
    EnableTimer->start();
}

void mainsystem::slot_EnableTime()
{
    if(MessageTimer.elapsed()<2000)
    {
        Error_Screen->setFixedSize(534,380); //System message size fix
        Error_Screen->show();
    }
    else if((2000<=MessageTimer.elapsed())&&(MessageTimer.elapsed()<2100))
    {
        Error_Screen->close();
        DisplayTimer->start();
        Enable_flag++;
        global::state_en[8] = false;
        global::state_en[9] = false;
        global::state_en[10] = false;
        global::state_en[0] = false;
    }
    if(Error_Screen->isHidden())
    {
        Enavble_True();
        Hide_Show_Display();
        global::qgauge_enable = true;
        ui->gaugeWidget_2->setForeground(Qt::yellow);
        ui->gaugeWidget_2->setThreshold(100);
        ui->gaugeWidget_2->setMinValue(0);
        ui->gaugeWidget_2->setMaxValue(130);
        Enable_flag++;
        if(Enable_flag >=3)
        {
            EnableTimer->stop();
            Enable_flag = 0;
        }
    }
    else
    {
        emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
        global::qgauge_enable = false;
        DisplayTimer->stop();
        Enavble_False();
    }
}
void mainsystem::Button_Move(mainsystem::RF_mode mode)
{
    if(mode==RF_MODE_TEMPERATURE)
    {
        ui->Up_button->move(310,310);
        ui->Down_button->move(310,460);
    }
    else
    {
        ui->Up_button->move(940,310);
        ui->Down_button->move(940,460);
    }

}
void mainsystem::slot_Setting_Enable()
{
        DisplayTimer->start();
        diplayTimer.start(50);
        Setting_Screen->hide();
        Setting_Screen->close();

        global::qgauge_visible=true;
        slot_Show();
        Modechange();
        Hide_Show_Display();
}

void mainsystem::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,20);
    QFont button_font(family,20);
    QFont CM_font(family,25);
    QFont symbol_font(family,15);
    QFont main_font(family,35);
    ui->Temp_symbol->setFont(button_font);
    ui->Time_symbol->setFont(label_font);
    ui->Watt_symbol->setFont(button_font);
    ui->Up_button->setFont(main_font);
    ui->Down_button->setFont(main_font);
    ui->ModeButton->setFont(button_font);
    ui->RF_Start_Stop->setFont(button_font);
    ui->Setting->setFont(button_font);
    ui->Time_symbol->setFont(symbol_font);
    ui->Modelabel->setFont(main_font);
    ui->CM_radio_Three->setFont(CM_font);
    ui->CM_radio_Seven->setFont(CM_font);
}
void mainsystem::Limited_Button(mainsystem::RF_mode mode)
{

    if((global::Watt[mode]==100)&&(mode==RF_MODE_CONTINUANCE))
    {
       ui->Up_button->setEnabled(false);
       ui->Down_button->setEnabled(true);
    }
    else if((global::Temp[mode]==120)&&(mode==RF_MODE_TEMPERATURE))
    {
        ui->Up_button->setEnabled(false);
        ui->Down_button->setEnabled(true);
    }
    else if((global::Watt[mode]==0)&&(mode==RF_MODE_CONTINUANCE))
    {
       ui->Up_button->setEnabled(true);
       ui->Down_button->setEnabled(false);
    }
    else if((global::Temp[mode]==60)&&(mode==RF_MODE_TEMPERATURE))
    {
        ui->Up_button->setEnabled(true);
        ui->Down_button->setEnabled(false);
    }
    else
    {
        ui->Up_button->setEnabled(true);
        ui->Down_button->setEnabled(true);
    }
}

void mainsystem::slot_Set_Display()
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    if (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
        emit sig_setting_display(RF_MODE_VENISTAR);
    else if(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)
        emit sig_setting_display(RF_MODE_CONTINUANCE);
    else
        emit sig_setting_display(RF_MODE_TEMPERATURE);
}
void mainsystem::Symbol_image_fix()
{
    ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/stop.png"));
    ui->startstop_symbol_label->setScaledContents(true);
    ui->Temp_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
    ui->Temp_symbol_label->setScaledContents(true);
    ui->Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
    ui->Watt_symbol_label->setScaledContents(true);
    ui->Time_symbol_label->setPixmap(QPixmap(":/Images/time.png"));
    ui->Time_symbol_label->setScaledContents(true);
    ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
    ui->Temp_Watt_symbol_label->setScaledContents(true);
}
void mainsystem::Hide_Show_Display()
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR))
    {
        ui->Up_button->show();
        ui->Down_button->show();
        ui->CM_radio_Seven->setEnabled(false);
        ui->CM_radio_Three->setEnabled(false);

    }
    else if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE))
    {
        ui->Up_button->show();
        ui->Down_button->show();
        ui->CM_radio_Seven->hide();
        ui->CM_radio_Three->hide();
    }
    else if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE))
    {
        ui->Up_button->show();
        ui->Down_button->show();
        ui->CM_radio_Seven->setEnabled(false);
        ui->CM_radio_Three->setEnabled(false);

    }
    else if (((getState() == STATE_POWER_READYTOACTIVE)||(getState() == STATE_SYSTEM_MODE_SETTING))&& (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR))
    {
        ui->Tempset->hide();
        ui->Temp_symbol_label->hide();
        ui->Up_button->hide();
        ui->Down_button->hide();
        ui->CM_radio_Seven->show();
        ui->CM_radio_Three->show();
        ui->CM_radio_Seven->setEnabled(true);
        ui->CM_radio_Three->setEnabled(true);
    }
    else if (((getState() == STATE_POWER_READYTOACTIVE)||(getState() == STATE_SYSTEM_MODE_SETTING))&&(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE))
    {
        ui->Tempset->hide();
        ui->Temp_symbol_label->hide();
        ui->Up_button->hide();
        ui->Down_button->hide();
        ui->CM_radio_Seven->hide();
        ui->CM_radio_Three->hide();

    }
    else if (((getState() == STATE_POWER_READYTOACTIVE)||(getState() == STATE_SYSTEM_MODE_SETTING))&& (Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE))
    {
        ui->Tempset->show();
        ui->Temp_symbol_label->show();
        ui->Up_button->hide();
        ui->Down_button->hide();
        ui->CM_radio_Seven->show();
        ui->CM_radio_Three->show();
        ui->CM_radio_Seven->setEnabled(true);
        ui->CM_radio_Three->setEnabled(true);

    }
}

void mainsystem::slot_Display()
{
    QPalette RF_start_stop_button=palette();
    QPalette Mode_button=palette();
    QPalette Setting_button=palette();
    QPalette palette = ui->Temp_symbol->palette();
    QPalette palette1 = ui->Watt_symbol->palette();
    QColor color = "white";
    palette.setColor(QPalette::WindowText,color);
    palette1.setColor(QPalette::WindowText,color);
    ui->Temp_symbol->setPalette(palette);
    ui->Watt_symbol->setPalette(palette1);
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    SetFont();
    if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)) //continu start
    {
        ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/stop.png"));
        ui->startstop_symbol_label->setScaledContents(true);
        RF_start_stop_button.setBrush(QPalette::Button,Qt::red);
        RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
        LCD_Display(RF_MODE_VENISTAR);
        Setting_Display(RF_MODE_VENISTAR);
        Button_Move(RF_MODE_VENISTAR);
        Limited_Button(RF_MODE_VENISTAR);
        ui->RF_Start_Stop->setText("RF Stop");
        ui->RF_Start_Stop->setPalette(RF_start_stop_button);
    }
    else if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE)) //continu start
    {

        ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/stop.png"));
        ui->startstop_symbol_label->setScaledContents(true);
        RF_start_stop_button.setBrush(QPalette::Button,Qt::red);
        RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
        LCD_Display(RF_MODE_CONTINUANCE);
        Setting_Display(RF_MODE_CONTINUANCE);
        Button_Move(RF_MODE_CONTINUANCE);
        Limited_Button(RF_MODE_CONTINUANCE);
        ui->RF_Start_Stop->setText("RF Stop");
        ui->RF_Start_Stop->setPalette(RF_start_stop_button);
    }
    else if((getState() == STATE_POWER_ACTIVE) && (Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)) //Temp start
    {
        ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/stop.png"));
        ui->startstop_symbol_label->setScaledContents(true);
        RF_start_stop_button.setBrush(QPalette::Button,Qt::red);
        RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
        LCD_Display(RF_MODE_TEMPERATURE);
        Setting_Display(RF_MODE_TEMPERATURE);
        Button_Move(RF_MODE_TEMPERATURE);
        Limited_Button(RF_MODE_TEMPERATURE);
        ui->RF_Start_Stop->setText("RF Stop");
        ui->RF_Start_Stop->setPalette(RF_start_stop_button);

    }
    else if ((getState() == STATE_POWER_READYTOACTIVE)&& (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR))
     {

         RF_start_stop_button.setBrush(QPalette::Button,Qt::blue);
         RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
         Mode_button.setBrush(QPalette::Button,Qt::green);
         Mode_button.setBrush(QPalette::ButtonText, Qt::black);
         Setting_button.setBrush(QPalette::Button,Qt::magenta);
         Setting_button.setBrush(QPalette::ButtonText, Qt::white);
         Setting_Display(RF_MODE_VENISTAR);
         LCD_Display(RF_MODE_VENISTAR);
         ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
         ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/start.png"));
         ui->Temp_Watt_symbol_label->setScaledContents(true);
         ui->startstop_symbol_label->setScaledContents(true);
         ui->RF_Start_Stop->setText("RF Start");
         ui->Modelabel->setText("Venistar");
         ui->RF_Start_Stop->setPalette(RF_start_stop_button);
         ui->ModeButton->setPalette(Mode_button);
         ui->Setting->setPalette(Setting_button);

         emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
         if(Venistar_CM_Mode == RF_DEVICE_OP_MODE_3CM)
         {
             ui->CM_radio_Three->setChecked(true);
             if(!Setting_CM_Mode)
             {
                 sysParm->tmRFPowerLevelPerModes[Mode] = 15; //watt
                 sysParm->tmRFCogluationTimePerModes[Mode] = 25; //sec
                 mRfDeviceFactors->rfPowerLevel = sysParm->tmRFPowerLevelPerModes[Mode];
                 sysParm->tmRFCogluationTime = sysParm->tmRFCogluationTimePerModes[Mode];
                 global::Watt[Mode] = mRfDeviceFactors->rfPowerLevel;
                 global::TimeS[Mode] = sysParm->tmRFCogluationTime;
             }
         }
         else
         {
             ui->CM_radio_Seven->setChecked(true);
             if(!Setting_CM_Mode)
             {
                 sysParm->tmRFPowerLevelPerModes[Mode] = 40; //watt
                 sysParm->tmRFCogluationTimePerModes[Mode] = 25; //sec
                 mRfDeviceFactors->rfPowerLevel = sysParm->tmRFPowerLevelPerModes[Mode];
                 sysParm->tmRFCogluationTime = sysParm->tmRFCogluationTimePerModes[Mode];
                 global::Watt[Mode] = mRfDeviceFactors->rfPowerLevel;
                 global::TimeS[Mode] = sysParm->tmRFCogluationTime;
             }
         }

     }
   else if ((getState() == STATE_POWER_READYTOACTIVE)&&(Mode == OutputMode::RF_DEVICE_OP_MODE_CONTINUANCE))
    {

        RF_start_stop_button.setBrush(QPalette::Button,Qt::blue);
        RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
        Mode_button.setBrush(QPalette::Button,Qt::yellow);
        Mode_button.setBrush(QPalette::ButtonText, Qt::black);
        Setting_button.setBrush(QPalette::Button,Qt::magenta);
        Setting_button.setBrush(QPalette::ButtonText, Qt::white);
        Setting_Display(RF_MODE_CONTINUANCE);
        LCD_Display(RF_MODE_CONTINUANCE);
        ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
        ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/start.png"));
        ui->Temp_Watt_symbol_label->setScaledContents(true);
        ui->startstop_symbol_label->setScaledContents(true);
        ui->RF_Start_Stop->setText("RF Start");
        ui->Modelabel->setText("Continuance");
        ui->RF_Start_Stop->setPalette(RF_start_stop_button);
        ui->ModeButton->setPalette(Mode_button);
        ui->Setting->setPalette(Setting_button);

        emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);

    }
    else if ((getState() == STATE_POWER_READYTOACTIVE)&&(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE))
     {
        Mode_button.setBrush(QPalette::Button,Qt::cyan);
        Mode_button.setBrush(QPalette::ButtonText, Qt::black);
         RF_start_stop_button.setBrush(QPalette::Button,Qt::blue);
         RF_start_stop_button.setBrush(QPalette::ButtonText, Qt::white);
         Setting_button.setBrush(QPalette::Button,Qt::magenta);
         Setting_button.setBrush(QPalette::ButtonText, Qt::white);
         Setting_Display(RF_MODE_TEMPERATURE);
         LCD_Display(RF_MODE_TEMPERATURE);
         ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
         ui->startstop_symbol_label->setPixmap(QPixmap(":/Images/start.png"));
         ui->Temp_Watt_symbol_label->setScaledContents(true);
         ui->startstop_symbol_label->setScaledContents(true);
         ui->RF_Start_Stop->setText("RF Start");
         ui->Modelabel->setText("Temperature");

         ui->RF_Start_Stop->setPalette(RF_start_stop_button);
         ui->ModeButton->setPalette(Mode_button);
         ui->Setting->setPalette(Setting_button);

         emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
         if(Temperature_CM_Mode == RF_DEVICE_OP_MODE_3CM)
         {
             ui->CM_radio_Three->setChecked(true);
             if(!Setting_CM_Mode)
             {
                sysParm->tmRFPowerLevelPerModes[Mode] = 10; //watt
                sysParm->tmRFCogluationTimePerModes[Mode] = 30; //sec
                mRfDeviceFactors->rfPowerLevel = sysParm->tmRFPowerLevelPerModes[Mode];
                sysParm->tmRFCogluationTime = sysParm->tmRFCogluationTimePerModes[Mode];
                global::Watt[Mode] = mRfDeviceFactors->rfPowerLevel;
                global::TimeS[Mode] = sysParm->tmRFCogluationTime;

             }
         }
         else
         {
             ui->CM_radio_Seven->setChecked(true);
             if(!Setting_CM_Mode)
             {
                sysParm->tmRFPowerLevelPerModes[Mode] = 20; //watt
                sysParm->tmRFCogluationTimePerModes[Mode] = 30; //sec
                mRfDeviceFactors->rfPowerLevel = sysParm->tmRFPowerLevelPerModes[Mode];
                sysParm->tmRFCogluationTime = sysParm->tmRFCogluationTimePerModes[Mode];
                global::Watt[Mode] = mRfDeviceFactors->rfPowerLevel;
                global::TimeS[Mode] = sysParm->tmRFCogluationTime;
             }
         }

     }
}

void mainsystem::Enavble_False()
{
    QPalette button_enable_false=palette();
    button_enable_false.setBrush(QPalette::Button,Qt::lightGray);
    button_enable_false.setBrush(QPalette::ButtonText, Qt::lightGray);
    ui->ModeButton->setPalette(button_enable_false);
    ui->Setting->setPalette(button_enable_false);
    ui->RF_Start_Stop->setPalette(button_enable_false);
    ui->Temp_symbol->setEnabled(false);
    ui->Time_symbol->setEnabled(false);
    ui->Watt_symbol->setEnabled(false);
    ui->line->setEnabled(false);
    ui->ModeButton->setEnabled(false);
    ui->Modelabel->setEnabled(false);
    ui->RF_Start_Stop->setEnabled(false);
    ui->Setting->setEnabled(false);
    ui->Tempset->setEnabled(false);
    ui->TimeLCD->setEnabled(false);
    ui->Timeset->setEnabled(false);
    ui->Wattset->setEnabled(false);
    ui->Up_button->setEnabled(false);
    ui->Down_button->setEnabled(false);
    ui->CM_radio_Three->setEnabled(false);
    ui->CM_radio_Seven->setEnabled(false);

}

void mainsystem::Enavble_True()
{
    ui->Temp_symbol->setEnabled(true);
    ui->Time_symbol->setEnabled(true);
    ui->Watt_symbol->setEnabled(true);
    ui->line->setEnabled(true);
    ui->ModeButton->setEnabled(true);
    ui->Modelabel->setEnabled(true);
    ui->RF_Start_Stop->setEnabled(true);
    ui->Setting->setEnabled(true);
    ui->Tempset->setEnabled(true);
    ui->TimeLCD->setEnabled(true);
    ui->Timeset->setEnabled(true);
    ui->Wattset->setEnabled(true);
    ui->CM_radio_Three->setEnabled(true);
    ui->CM_radio_Seven->setEnabled(true);

}

void mainsystem::slot_Hide()
{
    ui->Temp_symbol->hide();
    ui->Time_symbol->hide();
    ui->Watt_symbol->hide();
    ui->line->hide();
    ui->ModeButton->hide();
    ui->Up_button->hide();
    ui->Down_button->hide();
    ui->Modelabel->hide();
    ui->RF_Start_Stop->hide();
    ui->Setting->hide();
    ui->Tempset->hide();
    ui->TimeLCD->hide();
    ui->Timeset->hide();
    ui->Wattset->hide();
    ui->startstop_symbol_label->hide();
    ui->Time_symbol_label->hide();
    ui->Temp_symbol_label->hide();
    ui->Watt_symbol_label->hide();
    ui->Temp_Watt_symbol_label->hide();
    ui->colorr_label->hide();
    ui->colorr_label_2->hide();
    ui->CM_radio_Seven->hide();
    ui->CM_radio_Three->hide();
}

void mainsystem::slot_Show()
{
    ui->Temp_symbol->show();
    ui->Time_symbol->show();
    ui->Watt_symbol->show();
    ui->line->show();
    ui->ModeButton->show();
    ui->Up_button->show();
    ui->Down_button->show();
    ui->Modelabel->show();
    ui->RF_Start_Stop->show();
    ui->Setting->show();
    ui->Tempset->show();
    ui->TimeLCD->show();
    ui->Timeset->show();
    ui->Wattset->show();
    ui->startstop_symbol_label->show();
    ui->Time_symbol_label->show();
    ui->Temp_symbol_label->show();
    ui->Watt_symbol_label->show();
    ui->Temp_Watt_symbol_label->show();
    ui->colorr_label->show();
    ui->colorr_label_2->show();
    ui->CM_radio_Seven->show();
    ui->CM_radio_Three->show();
}

void mainsystem::slot_CM_Mode_Clicked()
{
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    if(ui->CM_radio_Three->isChecked()==true) // 3CM
    {
        if(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
        {
            Temperature_CM_Mode = RF_DEVICE_OP_MODE_3CM;
            outputThread->Temperature_CM_mode = RF_DEVICE_OP_MODE_3CM;
        }
        else if (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
        {
            Venistar_CM_Mode = RF_DEVICE_OP_MODE_3CM;
            outputThread->Venistar_CM_mode = RF_DEVICE_OP_MODE_3CM;
        }
    }
    else if(ui->CM_radio_Seven->isChecked()==true) // 7CM
    {
        if(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
        {
            Temperature_CM_Mode = RF_DEVICE_OP_MODE_7CM;
            outputThread->Temperature_CM_mode = RF_DEVICE_OP_MODE_7CM;
        }
        else if (Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
        {
            Venistar_CM_Mode = RF_DEVICE_OP_MODE_7CM;
            outputThread->Venistar_CM_mode = RF_DEVICE_OP_MODE_7CM;
        }
    }
}

void mainsystem::slot_RF_Button_Pushed()
{
    if(((Button_ElapsedTimer.elapsed()>=300))&&(getState() == STATE_POWER_READYTOACTIVE))
    {
        emit sig_btnLongPressed(BTNS::BTN_RFSTARTSTOP);
        return ;
    }
    else if((Button_ElapsedTimer.elapsed()<300)&&(getState() == STATE_POWER_ACTIVE)) //After 0.3s
    {
        emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
        return ;
    }
    if(ui->RF_Start_Stop->isDown() == true)
    {
        QTimer::singleShot(100,this,SLOT(slot_RF_Button_Pushed()));
    }
}

void mainsystem::installFilter()
{
    //this->installEventFilter(this);
    ui->ModeButton->installEventFilter(this);
    ui->RF_Start_Stop->installEventFilter(this);
    ui->Setting->installEventFilter(this);
    ui->Up_button->installEventFilter(this);
    ui->Down_button->installEventFilter(this);
}

bool mainsystem::eventFilter(QObject *target, QEvent *event)
{ 
    OutputMode::RF_Output_Mode_en Mode = outputThread->GetOutputMode();
    if((target == ui->ModeButton) && (event->type() == QEvent::MouseButtonPress))
    {
        if( getState()==STATE_POWER_READYTOACTIVE)//global::Start==RF_Stop
        {
           emit sig_btnReleased(BTNS::BTN_MODE);
           Setting_Screen->slot_Mode_Change();
        }
        Hide_Show_Display();
        Mode_flag = 0;
    }
    else if((target == ui->RF_Start_Stop) && (event->type() == QEvent::MouseButtonPress))
    {
        Button_ElapsedTimer.restart();
        QTimer::singleShot(100,this,SLOT(slot_RF_Button_Pushed()));

    }
    else if((target == ui->Setting) && (event->type() == QEvent::MouseButtonPress))
    {
        if( getState()==STATE_POWER_READYTOACTIVE)//global::Start==RF_Stop
         {
            if(Mode == OutputMode::RF_DEVICE_OP_MODE_VENISTAR)
            {
               Setting_CM_Mode = true;
            }

            else if(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
            {
               Setting_CM_Mode = true;
            }
           emit sig_btnReleased(BTNS::BTN_ENCORDER_SW);
           global::qgauge_visible=false;
           DisplayTimer->stop();
           diplayTimer.stop();
           slot_Hide();
           Setting_Screen->show();
        }
    }
    else if((target == ui->Up_button) && (event->type() == QEvent::MouseButtonPress))
    {
        if(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
        {
                global::state[6]=true;
        }
        else
        {
                global::state[4]=true;
        }
         Step[0] = true;
         emit sig_btnDirReleased(BTNS::BTN_DR_RIGHT);
    }
    else if((target == ui->Down_button) && (event->type() == QEvent::MouseButtonPress))
    {
        if(Mode == OutputMode::RF_DEVICE_OP_MODE_TEMPERATURE)
        {
                global::state[7]=true;
        }
        else
        {
                global::state[5]=true;
        }
         Step[1] = true;
         emit sig_btnDirReleased(BTNS::BTN_DR_LEFT);
    }
    return QWidget::eventFilter(target, event);
}
#ifndef __FEATURE_VVR12_BOARD__
/////////////font change code////////////////////////
/*
QString family = QFontDatabase::applicationFontFamilies(id_reg).at(0);

QFont openSansReg(family, 15);


ui->label->setFont( openSansReg );
ui->label->setText("ON");

QString family = QFontDatabase::applicationFontFamilies(id_semi).at(0);

QFont openSansSemi(family, 20);

ui->label->setFont( openSansSemi );

//ui->label->setText("H");
ui->label->setText("OFF");

*/
#endif
#ifndef __FEATURE_VVR12_BOARD__

/////////////////////////////////////////////////////
///test////////////////////////////////////////////////////////
void mainsystem::SysMessage_clicked()
{
   //emit sig_Sysmessage(STATE_SYSTEM_FAIL);
   // emit sig_Sysmessage(STATE_ERROR_OVER_IMPEDANCE);
   //emit sig_Sysmessage(STATE_ERROR_OVER_TEMPERATURE);
    emit sig_Sysmessage(STATE_ERROR_LOW_IMPEDANCE);
   // emit sig_init();
}
//////////////////////////////////////////////////////////
#endif
