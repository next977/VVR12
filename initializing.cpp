#include "initializing.h"
#include "ui_initializing.h"
#include"mainsystem.h"
#include <QFontDatabase>
#include <QDebug>
#include <QFont>


initializing::initializing(QWidget *parent, SysParm *sysParm) :
    QWidget(parent),
    ui(new Ui::initializing)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    timercount = new QTimer(this);
    this->sysParm = sysParm;
    int id = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    this->setIDstarmed(id);

    ui->Serialno_symbol_label->setPixmap(QPixmap(":/Images/serialno.png"));
    ui->Serialno_symbol_label->setScaledContents(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(Timer_state())); //PushTimer
    connect(timercount, SIGNAL(timeout()), this, SLOT(Timer_counter())); //PushTimer
    Dispaly_Hide();
#ifndef __FEATURE_VVR12_BOARD__
   // timer->start(500);
   // timercount->start();
   // elapsedtimer.start();
   // waitt_elapsedtimer.start();
#endif
}
initializing::~initializing()
{
    delete ui;
}
void initializing::Timer_state()
{
    QString Serial_No_Print;
    Serial_No_Print.sprintf("%s",(int8*)SERIAL_NO);
    ui->serial_label->setText(Serial_No_Print);
    ui->V_LCD->display(sysParm->mRfDeviceFactors->tmReadADCVoltage);
    ui->I_LCD->display(sysParm->mRfDeviceFactors->tmReadADCCurrent);
    ui->T_LCD->display(sysParm->mRfDeviceFactors->tmReadADCTemperatureA);
    ui->W_LCD->display(sysParm->mRfDeviceFactors->rfPowerMeasured/100);
    ui->R_LCD->display(sysParm->mRfDeviceFactors->rfImpedanceMeasured);
    //ui->W_LCD->display(sysParm->mRfDeviceFactors[0].rfPowerMeasured/100);
    //ui->R_LCD->display(sysParm->mRfDeviceFactors[0].rfImpedanceMeasured);

    for(int16 i = 0 ; i <5 ; i ++)
    {
        if(sysParm->mRfDeviceFactors->System_result == true)
        {
            Dispaly_State_Ok(i);
        }
        else
        {

            Dispaly_State_Fail(i);
        }
    }
    if(global::System_Initializing_visible==true)
    {
        Dispaly_Show(i);
#ifndef __FEATURE_VVR12_BOARD__
        if(global::Init_State[i]==false)
        {
           // while(true);
            qDebug("fail %d", i);
           // forever{;}
        }
           // waitt_elapsedtimer.restart();
           // while(waitt_elapsedtimer.elapsed()<=500) ;
#endif
        if(i==5)
        {
            i = 0;
            global::System_Initializing_visible=false;
        }
        i++;
    }
}
void initializing::Dispaly_State_Ok(int16 state)
{
    switch(state)
    {
        case STATE_V_OK :
            ui->V_state_label->setText("OK");
            break;
        case STATE_I_OK :
            ui->I_state_label->setText("OK");
            break;
        case STATE_T_OK :
            ui->T_state_label->setText("OK");
            break;
        case STATE_W_OK :
            ui->W_state_label->setText("OK");
            break;
        case STATE_R_OK :
            ui->R_state_label->setText("OK");
            break;
        default :
            break;
    }
}
void initializing::Dispaly_State_Fail(int16 state)
{
    switch(state)
    {
        case STATE_V_Fail :
            ui->V_state_label->setText("Fail");
            break;
        case STATE_I_Fail :
            ui->I_state_label->setText("Fail");
            break;
        case STATE_T_Fail :
            ui->T_state_label->setText("Fail");
            break;
        case STATE_W_Fail :
            ui->W_state_label->setText("Fail");
            break;
        case STATE_R_Fail :
            ui->R_state_label->setText("Fail");
            break;
        default :
            break;
    }
}
void initializing::Dispaly_Show(int32 i)
{
    switch(i)
    {
        case 0 :
            ui->V_label->show();
            ui->V_LCD->show();
            ui->V_default->show();
            ui->V_state_label->show();
            break;
        case 1 :
            ui->I_label->show();
            ui->I_LCD->show();
            ui->I_default->show();
            ui->I_state_label->show();
            break;
        case 2 :
            ui->T_label->show();
            ui->T_LCD->show();
            ui->T_default->show();
            ui->T_state_label->show();
            break;
        case 3 :
            ui->W_label->show();
            ui->W_LCD->show();
            ui->W_default->show();
            ui->W_state_label->show();
            break;
        case 4 :
            ui->R_label->show();
            ui->R_LCD->show();
            ui->R_default->show();
            ui->R_state_label->show();
            break;
        default :
            break;
    }
}
void initializing::Dispaly_Hide()
{
    ui->V_label->hide();
    ui->V_LCD->hide();
    ui->V_default->hide();
    ui->V_state_label->hide();
    ui->I_label->hide();
    ui->I_LCD->hide();
    ui->I_default->hide();
    ui->I_state_label->hide();
    ui->T_label->hide();
    ui->T_LCD->hide();
    ui->T_default->hide();
    ui->T_state_label->hide();
    ui->W_label->hide();
    ui->W_LCD->hide();
    ui->W_default->hide();
    ui->W_state_label->hide();
    ui->R_label->hide();
    ui->R_LCD->hide();
    ui->R_default->hide();
    ui->R_state_label->hide();

}
void initializing::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,25);
    QFont Seriallabel_font(family,20);
    QFont main_font(family,40);

    ui->V_label->setFont(label_font);
    ui->V_state_label->setFont(label_font);
    ui->V_default->setFont(label_font);
    ui->I_label->setFont(label_font);
    ui->I_state_label->setFont(label_font);
    ui->I_default->setFont(label_font);
    ui->T_label->setFont(label_font);
    ui->T_state_label->setFont(label_font);
    ui->T_default->setFont(label_font);
    ui->W_label->setFont(label_font);
    ui->W_state_label->setFont(label_font);
    ui->W_default->setFont(label_font);
    ui->R_label->setFont(label_font);
    ui->R_state_label->setFont(label_font);
    ui->R_default->setFont(label_font);
    ui->serial_label->setFont(Seriallabel_font);
    ui->Modelabel->setFont(main_font);
}


void initializing::Timer_counter()
{
    SetFont();
    if((elapsedtimer.elapsed()>=5000))//stopWatch.elapsed()>=500
    {
        timercount->stop();
        emit sig_init();
    }
}
#ifndef __FEATURE_VVR12_BOARD__

void Thread::run()
{


    forever
    {

        if((global11::Start==true) && (Mode==true)) //continu start
        {
            QElapsedTimer Counttimer;
            Counttimer.start();
            qDebug("Thread continu Start!!!!!");
            while(Counttimer.elapsed()<1000) ; //1sec delay
            if(global11::Start == false) return; //Thread stop
            if(Watt2[0] < global1::Watt[0])  Watt2[0] +=5;
            if(TimeM2[0] < global1::TimeM[0])
            {
                 if(TimeS2[0] < 59)  TimeS2[0]++;
                 else
                 {
                      TimeS2[0] = 0;
                      TimeM2[0] ++;
                 }
             }
             else
             {
                 if(TimeS2[0] < global1::TimeS[0])
                 {
                    TimeS2[0]++;
                 }
                 else global11::Start = false;
             }
        }
        else if((global11::Start==true) && (Mode==false)) //Temp start
        {
            QElapsedTimer Counttimer;
            Counttimer.start();
            qDebug("Thread Temp Start!!!!!");
            while(Counttimer.elapsed()<1000) ; //1sec delay
            if(global11::Start == false) return; //Thread stop
            if(Watt2[1] < global1::Watt[1]) Watt2[1] +=5;
            if(Temp2[1] < global1::Temp[1]) Temp2[1] +=10;
            if(TimeM2[1] < global1::TimeM[1])
            {
                if(TimeS2[1] < 59)
                {
                    TimeS2[1]++;
                }
                else
                {
                     TimeS2[1] = 0;
                     TimeM2[1] ++;
                }
            }
            else
            {
                if(TimeS2[1] < (global1::TimeS[1])) TimeS2[1]++;
                else global11::Start = false;
            }
        }

    }

}

#endif
