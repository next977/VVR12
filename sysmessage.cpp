#include "sysmessage.h"
#include "ui_sysmessage.h"
#include "mainsystem.h"
#include <QPainter>
#include <QFontDatabase>
#include <QDebug>
#include <QFont>

sysmessage::sysmessage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sysmessage)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    int id = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    this->setIDstarmed(id);
    ui->Danger_symbol->setPixmap(QPixmap(":/Images/danger.png"));
    ui->Danger_symbol->setScaledContents(true);
    ui->Action_symbol->setPixmap(QPixmap(":/Images/action.png"));
    ui->Action_symbol->setScaledContents(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(Timer_state())); //PushTimer
    timer->start();
}

sysmessage::~sysmessage()
{
    delete ui;
}
void sysmessage::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,20);
    QFont Action_label_font(family,15);

  //  QFont button_font(family,15);
   // QFont symbol_font(family,11);
    QFont main_font(family,25);
    ui->Code_No_label->setFont(label_font);
    ui->Code_label->setFont(label_font);
    ui->System_label->setFont(main_font);
    ui->Action_label->setFont(label_font);
    ui->Action_No_label->setFont(Action_label_font);
}

void sysmessage::Timer_state()
{
    State_en state;
    SetFont();
    //if(global::state_en[0]==true)  state = STATE_SYSTEM_FAIL;
    if(global::state_en[8]==true)  state = STATE_ERROR_OVER_TEMPERATURE;
    else if(global::state_en[9]==true)  state = STATE_ERROR_OVER_IMPEDANCE;
    else if(global::state_en[10]==true)  state = STATE_ERROR_LOW_IMPEDANCE;
    else state = STATE_ERROR_None;
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
        ui->Code_No_label->setText("003");
        ui->System_label->setText("Temperature Over");
        //ui->Action_No_label->setText("Please lower the temperature of the electrode");
        ui->Action_No_label->setText("Please check temperature set point");
        break;

    case STATE_ERROR_OVER_IMPEDANCE:
        ui->Code_No_label->setText("001");
        ui->System_label->setText("Impedance High");
        ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    case STATE_ERROR_LOW_IMPEDANCE:
        ui->Code_No_label->setText("002");
        ui->System_label->setText("Impedance Low");
        ui->Action_No_label->setText("Please check the connection of electrode");
        break;
    /*
    case STATE_ERROR_OVER_IMPEDANCE:
        ui->Code_No_label->setText("001");
        ui->System_label->setText("Impedance High");
        ui->Action_No_label->setText("Please check the connection of electrode");
        global::state_en[9] = false;
        break;
    case STATE_ERROR_NeedTimeSec:
        ui->System_label->setText("Need TimeSec");
        global::state_en[10] = false;
        break;
    */
    case STATE_SYSTEM_FAIL:
        ui->Code_No_label->setText("003");
        ui->System_label->setText("System Fail");
        break;
    default:
        ui->Code_No_label->setText("000");
        ui->System_label->setText("None");
        ui->Action_No_label->setText("None");
        break;
    }

}

void sysmessage::paintEvent(QPaintEvent *event) //위젯이나 다이얼로그의 테두리 그리기
{
    QPainter painter(this);
    //인자 설명 - 시작좌표x,시작좌표y,현재화면크기(길이),현재화면크기(높이)  0,0은 가장자리의 곡선설정
    painter.drawRoundedRect(0,0,width()-1,height()-1,0,0);
    QDialog::paintEvent(event);
}
