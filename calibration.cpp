#include "calibration.h"
#include "ui_calibration.h"
#include "mainsystem.h"
#include <QFontDatabase>
#include <QDebug>
#include <QFont>

calibration::calibration(QWidget *parent, SysParm *sysParm) :
    QWidget(parent),
    ui(new Ui::calibration)
{
    ui->setupUi(this);
    Maintimer = new QTimer(this);
    Maintimer->setInterval(30);


    this->sysParm = sysParm;
    int id = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    this->setIDstarmed(id);
    CalibrationMode();
    QPalette RF_button=palette();

    RF_button.setBrush(QPalette::Button,Qt::blue);
    RF_button.setBrush(QPalette::ButtonText, Qt::white);
    ui->RF_button->setPalette(RF_button);

    connect(Maintimer,SIGNAL(timeout()), this, SLOT(slot_Main_Display()));
    installFilter();
    Maintimer->start();

}

calibration::~calibration()
{
    delete ui;
}

void calibration::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,25);
    QFont button_font(family,30);
    QFont main_font(family,30);
    ui->Modelabel->setFont(main_font);
    ui->Back_button->setFont(button_font);
    ui->Up_button->setFont(button_font);
    ui->Down_button->setFont(button_font);
    ui->mode_button->setFont(button_font);
    ui->RF_button->setFont(button_font);
    ui->DAC_label->setFont(label_font);
    ui->V_label->setFont(label_font);
    ui->T_A_label->setFont(label_font);
    ui->T_B_label->setFont(label_font);
    ui->I_label->setFont(label_font);
    ui->W_label->setFont(label_font);
    ui->R_label->setFont(label_font);
    ui->sw_label->setFont(label_font);
    ui->SN_label->setFont(label_font);
    ui->SW_label->setFont(label_font);
    ui->serial_label->setFont(label_font);
}


void calibration::slot_Main_Display()
{

    QString Serial_No_Print;
    QString SW_Print;
    Serial_No_Print.sprintf("%s",(int8*)SERIAL_NO);
    SW_Print.sprintf("%s",(int8*)SW_VER);
    ui->serial_label->setText(Serial_No_Print);
    ui->sw_label->setText(SW_Print);
    SetFont();
    //ui->DAC_LCD->display(sysParm->mRfDeviceFactors->tmDACValueToSetLast);
    ui->DAC_LCD->display(sysParm->mRfDeviceFactors->tmDACValueToSetforCal);
    ui->V_LCD->display(sysParm->mRfDeviceFactors->tmReadADCVoltage);
    ui->T_A_LCD->display(sysParm->mRfDeviceFactors->tmReadADCTemperatureA);
    ui->T_B_LCD->display(sysParm->mRfDeviceFactors->tmReadADCTemperatureB);
    ui->I_LCD->display(sysParm->mRfDeviceFactors->tmReadADCCurrent);
    ui->W_LCD->display(sysParm->mRfDeviceFactors->rfPowerMeasured/100);
    ui->R_LCD->display(sysParm->mRfDeviceFactors->rfImpedanceMeasured);


}

void calibration::CalibrationMode()
{
    sysParm->mRfDeviceFactors->tmDACValueToSetLast = 0;
    data_in_index = 0;
    connect(this, SIGNAL(sig_btnReleased(int)), this, SLOT(slot_btnReleasedCal(int)));
    connect(this, SIGNAL(sig_btnDirReleased(int)), this, SLOT(slot_btnDirReleasedCal(int)));
}

void calibration::slot_btnReleasedCal(int keyEvent)
{
    QPalette RF_button=palette();

    switch(keyEvent)
    {

        case BTNS::BTN_RFSTARTSTOP:
            if(!sysParm->isDoTestforCal )
            {
                qDebug("Enable Amp");
                RF_button.setBrush(QPalette::Button,Qt::red);
                RF_button.setBrush(QPalette::ButtonText, Qt::white);
                ui->RF_button->setPalette(RF_button);

                data_in_index = 0;
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal = 0;
                sysParm->mRfDeviceFactors->tmDACValueToSetLast =  0;
                sysParm->isDoTestforCal = true;
                gpioTools::EnableAmplifierPowerOutput();
                gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_TARGET);
                gpioTools::SetElectrodeType(gpioTools::ELECTRODE_MONOPOLAR);
            }
            else
            {
                qDebug("Disnable Amp");
                RF_button.setBrush(QPalette::Button,Qt::blue);
                RF_button.setBrush(QPalette::ButtonText, Qt::white);
                ui->RF_button->setPalette(RF_button);

                sysParm->mRfDeviceFactors->tmDACValueToSetLast =  0;
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal = 0;
                gpioTools::DACPortOutput(sysParm->mRfDeviceFactors->tmDACValueToSetLast);

                gpioTools::SetElectrodeType(gpioTools::ELECTRODE_NONE);
                gpioTools::SetAmplifierPowerOutputDirection(gpioTools::RFOUTPUT_DIR_TARGET);
                gpioTools::DisableAmplifierPowerOutput();
                sysParm->isDoTestforCal = false;
            }
            break;

        case BTNS::BTN_MODE:
            qDebug("BTN_USB");

#ifdef _ONPC_
            printf("%d,%d,%d,%d,%d\n",
                    data_in_index++,
                    sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                    sysParm->mRfDeviceFactors->tmReadADCVoltage,
                    sysParm->mRfDeviceFactors->tmReadADCCurrent,
                    sysParm->mRfDeviceFactors->tmReadADCTemperatureA,
                    sysParm->mRfDeviceFactors->tmReadADCTemperatureB);
#else
            gpioTools::uart_printf("%d,%d,%d,%d,%d\n",
                    data_in_index++,
                    sysParm->mRfDeviceFactors->tmDACValueToSetforCal,
                    sysParm->mRfDeviceFactors->tmReadADCVoltage,
                    sysParm->mRfDeviceFactors->tmReadADCCurrent,
                    sysParm->mRfDeviceFactors->tmReadADCTemperatureA,
                    sysParm->mRfDeviceFactors->tmReadADCTemperatureB );
#endif
            //StopSound();
            //StartSound(WAVE_keytmp);
            break;

        default:
            break;
    }
}

void calibration::slot_btnDirReleasedCal(int keyEvent)
{
    switch( keyEvent )
    {
        case BTNS::BTN_DR_RIGHT:
            sysParm->mRfDeviceFactors->tmDACValueToSetforCal+=5;
            if(sysParm->isDoTestforCal)sysParm-> mRfDeviceFactors->tmDACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            break;

        case BTNS::BTN_DR_LEFT:
            if(sysParm->mRfDeviceFactors->tmDACValueToSetforCal<=0)
            {
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal=0;
            }
            else
                sysParm->mRfDeviceFactors->tmDACValueToSetforCal-=5;
            if(sysParm->isDoTestforCal) sysParm->mRfDeviceFactors->tmDACValueToSetLast = sysParm->mRfDeviceFactors->tmDACValueToSetforCal;
            break;

        default:
            break;
    }
}
void calibration::slot_Up_Button_Push()
{
   if(ui->Up_button->isDown() == true)
   {
       emit sig_btnDirReleased(BTNS::BTN_DR_RIGHT);
       QTimer::singleShot(100,this,SLOT(slot_Up_Button_Push()));
   }
}
void calibration::slot_Down_Button_Push()
{
    if(ui->Down_button->isDown() == true)
    {
        emit sig_btnDirReleased(BTNS::BTN_DR_LEFT);
        QTimer::singleShot(100,this,SLOT(slot_Down_Button_Push()));
    }
}

void calibration::installFilter()
{
    ui->RF_button->installEventFilter(this);
    ui->Back_button->installEventFilter(this);
    ui->mode_button->installEventFilter(this);
    ui->Up_button->installEventFilter(this);
    ui->Down_button->installEventFilter(this);
}
bool calibration::eventFilter(QObject *target, QEvent *event)
{
    if((target == ui->RF_button) && (event->type() == QEvent::MouseButtonPress))
    {
       emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
    }
    else if((target == ui->Back_button) && (event->type() == QEvent::MouseButtonPress))
    {
        global::cal = false;
        if(sysParm->isDoTestforCal == true)
        {
            emit sig_btnReleased(BTNS::BTN_RFSTARTSTOP);
        }
        emit sig_calibration();
    }
    else if((target == ui->mode_button) && (event->type() == QEvent::MouseButtonPress))
    {
        emit sig_btnReleased(BTNS::BTN_MODE);
    }
    else if((target == ui->Up_button) && (event->type() == QEvent::MouseButtonPress))
    {
         QTimer::singleShot(100,this,SLOT(slot_Up_Button_Push()));
         emit sig_btnDirReleased(BTNS::BTN_DR_RIGHT);
    }
    else if((target == ui->Down_button) && (event->type() == QEvent::MouseButtonPress))
    {
         QTimer::singleShot(100,this,SLOT(slot_Down_Button_Push()));
         emit sig_btnDirReleased(BTNS::BTN_DR_LEFT);
    }

    return QWidget::eventFilter(target, event);
}
