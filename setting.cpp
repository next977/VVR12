#include "setting.h"
#include "ui_setting.h"
#include "mainsystem.h"
#include <QFontDatabase>
#include <QDebug>
#include <QFont>
setting::setting(QWidget *parent, SysParm *sysParm) :
    QWidget(parent),
    ui(new Ui::setting)
{
     ui->setupUi(this);
    this->sysParm = sysParm;

    Calibration_Screen= new calibration(this,sysParm);

    Maintimer = new QTimer(this);
    Subtimer = new QTimer(this);

    EnableTimer = new QTimer(this);

    Maintimer->setInterval(30);

    QPalette complete_button=palette();
    complete_button.setBrush(QPalette::Button,Qt::blue);
    complete_button.setBrush(QPalette::ButtonText, Qt::white);
    ui->BackButton->setPalette(complete_button);

    setWindowFlags(Qt::FramelessWindowHint); // frame
    Setting_Display(RF_MODE_TEMPERATURE);
    mode = RF_MODE_TEMPERATURE;
    global::Click_state[0] = Click_Watt;
    global::Click_state[1] = Click_Watt;
    global::Click_state[2] = Click_Temp;

    ui->Temp_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
    ui->Temp_symbol_label->setScaledContents(true);
    ui->Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
    ui->Watt_symbol_label->setScaledContents(true);
    ui->Time_symbol_label->setPixmap(QPixmap(":/Images/time.png"));
    ui->Time_symbol_label->setScaledContents(true);
    ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
    ui->Temp_Watt_symbol_label->setScaledContents(true);

    int id = QFontDatabase::addApplicationFont(":/Fonts/starmedfont.ttf");
    this->setIDstarmed(id);

    connect(Calibration_Screen,SIGNAL(sig_calibration()), this, SLOT(slot_Sub()));

    /////////////////////////Timer ///////////////////////////////////////
    connect(Maintimer, SIGNAL(timeout()), this, SLOT(display())); //Timer
    connect(Subtimer, SIGNAL(timeout()), this, SLOT(slot_Sub())); //Timer
    Subtimer->start();
    Maintimer->start();
    Calibration_Screen->hide();
    installFilter();
}

setting::~setting()
{
    delete ui;
}


void setting::Setting_Move(setting::RF_mode mode)
{
    if(mode==RF_MODE_TEMPERATURE)
    {
        ui->WattLCD->move(350,300);
        ui->Watt_symbol->move(600,310);
        ui->Time_symbol->move(600,460);
        ui->TimeMLCD->move(230,460);
        ui->TimeLCD_label->move(300,460);
        ui->TimeSLCD->move(400,460);
        ui->Watt_line->move(410,390);
        ui->TimeM_line->move(250,550);
        ui->TimeS_line->move(410,550);
        ui->Temp_symbol_label->move(80,160);
        ui->Watt_symbol_label->move(80,320);
        ui->Time_symbol_label->move(70,480);
    }
    else
    {
        ui->WattLCD->move(350,130);
        ui->Watt_symbol->move(600,150);
        ui->Time_symbol->move(600,300);
        ui->TimeMLCD->move(230,300);
        ui->TimeLCD_label->move(300,300);
        ui->TimeSLCD->move(400,300);
        ui->Watt_line->move(410,220);
        ui->TimeM_line->move(250,390);
        ui->TimeS_line->move(410,390);
        ui->Watt_symbol_label->move(80,160);
        ui->Time_symbol_label->move(70,320);
    }

}
void setting::Setting_Display(setting::RF_mode mode)
{
    QString TimeM;
    QString TimeS;
    TimeM.sprintf("%02d", global::TimeM[mode]);
    TimeS.sprintf("%02d", global::TimeS[mode]);
    global::Display.sprintf("%02d:%02d", global::TimeM[mode], global::TimeS[mode]);

    if(mode==RF_MODE_VENISTAR)
    {
        ui->TempLCD->hide();
        ui->Temp_symbol_label->hide();
        Setting_Move(RF_MODE_VENISTAR);

    }
    else if(mode==RF_MODE_CONTINUANCE)
    {
        ui->TempLCD->hide();
        ui->Temp_symbol_label->hide();
        Setting_Move(RF_MODE_CONTINUANCE);

    }
    else
    {
        ui->TempLCD->show();
        ui->Temp_symbol_label->show();
        ui->TempLCD->display(global::Temp[mode]);
        Setting_Move(RF_MODE_TEMPERATURE);

    }
    ui->TimeMLCD->display(TimeM);
    ui->TimeSLCD->display(TimeS);
    ui->TimeLCD_label->display(":");
    ui->WattLCD->display(global::Watt[mode]);
    Limited_Button(mode);
}
void setting::Limited_Button(uint16 mode)
{
    if((global::Watt[mode]==100)&&(global::Click_state[mode] == Click_Watt))
    {
       ui->Up_button->setEnabled(false);
       ui->Down_button->setEnabled(true);
    }
    else if((global::Temp[mode]==120)&&(global::Click_state[mode] == Click_Temp))
    {
        ui->Up_button->setEnabled(false);
        ui->Down_button->setEnabled(true);
    }
    else if((global::TimeM[mode]==59)&&(global::Click_state[mode] == Click_TimeMin))
    {
        ui->Up_button->setEnabled(false);
        ui->Down_button->setEnabled(true);
    }
    else if((global::TimeS[mode]==59)&&(global::Click_state[mode] == Click_TimeSec))
    {
        ui->Up_button->setEnabled(false);
        ui->Down_button->setEnabled(true);
    }
    else if((global::Watt[mode]==0)&&(global::Click_state[mode] == Click_Watt))
    {
       ui->Up_button->setEnabled(true);
       ui->Down_button->setEnabled(false);
    }
    else if((global::Temp[mode]==60)&&(global::Click_state[mode] == Click_Temp))
    {
        ui->Up_button->setEnabled(true);
        ui->Down_button->setEnabled(false);
    }
    else if((global::TimeM[mode]==0)&&(global::Click_state[mode] == Click_TimeMin))
    {
        ui->Up_button->setEnabled(true);
        ui->Down_button->setEnabled(false);
    }
    else if((global::TimeS[mode]==0)&&(global::Click_state[mode] == Click_TimeSec))
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

void setting::SetFont()
{
    QString family = QFontDatabase::applicationFontFamilies(id_starmed).at(0);
    QFont label_font(family,25);
    QFont button_font(family,20);
    QFont main_font(family,35);
    ui->Temp_symbol->setFont(label_font);
    ui->Time_symbol->setFont(label_font);
    ui->Watt_symbol->setFont(label_font);
    ui->ModeButton->setFont(button_font);
    ui->BackButton->setFont(button_font);
    ui->Up_button->setFont(main_font);
    ui->Down_button->setFont(main_font);
    ui->Modelabel->setFont(main_font);
}
void setting::Choose_Click_state(setting::RF_mode mode)
{
    switch(global::Click_state[mode])
    {
        case Click_None:
            ui->TimeM_line->hide();
            ui->TimeS_line->hide();
            ui->Watt_line->hide();
            ui->Temp_line->hide();
            break;

        case Click_TimeMin:
            ui->TimeM_line->show();
            ui->TimeS_line->hide();
            ui->Watt_line->hide();
            ui->Temp_line->hide();
            break;

        case Click_TimeSec:
            ui->TimeM_line->hide();
            ui->TimeS_line->show();
            ui->Watt_line->hide();
            ui->Temp_line->hide();
            break;

        case Click_Watt:
            ui->TimeM_line->hide();
            ui->TimeS_line->hide();
            ui->Watt_line->show();
            ui->Temp_line->hide();
            break;

        case Click_Temp:
            ui->TimeM_line->hide();
            ui->TimeS_line->hide();
            ui->Watt_line->hide();
            ui->Temp_line->show();
            break;

        default:
            break;
   }
}
void setting::slot_Sub()
{
    if(global::cal == true)
    {
        Maintimer->stop();
        Calibration_Screen->show();
        Hide();
        Subtimer->stop();

    }
    else
    {
         Maintimer->start();
         Calibration_Screen->hide();
         Show();
         Subtimer->stop();
    }

}
void setting::display()
{
    QPalette Display=palette();
    SetFont();
    if(mode == RF_MODE_VENISTAR) // venistar mode
    {
        Display.setBrush(QPalette::Button,Qt::green);
        Display.setBrush(QPalette::ButtonText, Qt::black);
        ui->ModeButton->setPalette(Display);
        ui->Modelabel->setText("Venistar");
        ui->Temp_symbol->setText("");
        Setting_Display(RF_MODE_VENISTAR);
        Choose_Click_state(RF_MODE_VENISTAR);
        ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
        ui->Temp_Watt_symbol_label->setScaledContents(true);
    }
    else if(mode == RF_MODE_CONTINUANCE) // continu mode
    {
        Display.setBrush(QPalette::Button,Qt::yellow);
        Display.setBrush(QPalette::ButtonText, Qt::black);
        ui->ModeButton->setPalette(Display);
        ui->Modelabel->setText("Continuance");
        ui->Temp_symbol->setText("");
        Setting_Display(RF_MODE_CONTINUANCE);
        Choose_Click_state(RF_MODE_CONTINUANCE);
        ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/temp.png"));
        ui->Temp_Watt_symbol_label->setScaledContents(true);
    }
    else
    {
        Display.setBrush(QPalette::Button,Qt::cyan);
        Display.setBrush(QPalette::ButtonText, Qt::black);
        ui->ModeButton->setPalette(Display);
        ui->Modelabel->setText("Temperature");
        ui->Temp_symbol->setText("ºc");
        Setting_Display(RF_MODE_TEMPERATURE);
        Choose_Click_state(RF_MODE_TEMPERATURE);
        ui->Temp_Watt_symbol_label->setPixmap(QPixmap(":/Images/watt.png"));
        ui->Temp_Watt_symbol_label->setScaledContents(true);
    }
}
void setting::slot_Up_Button_Push()
{
    if(ui->Up_button->isDown() == true)
    {
        if(mode == RF_MODE_VENISTAR)
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::Push_state[6]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[0]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[1]=true;
            }

        }
        else if(mode == RF_MODE_CONTINUANCE)
        {

            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::Push_state[6]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[0]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[1]=true;
            }

        }
        else
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::Push_state[6]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[0]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[1]=true;
            }
        }
         emit sig_btnDirReleased(BTNS::BTN_DR_RIGHT);
        QTimer::singleShot(200,this,SLOT(slot_Up_Button_Push()));
   }

}

void setting::slot_Down_Button_Push()
{
    if(ui->Down_button->isDown() == true)
    {
        if(mode == RF_MODE_VENISTAR)
        {

            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::Push_state[7]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[3]=true;
            }
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {

            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::Push_state[7]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[3]=true;
            }

        }
        else
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
               global::Push_state[7]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::Push_state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::Push_state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::Push_state[3]=true;
            }
        }
         emit sig_btnDirReleased(BTNS::BTN_DR_LEFT);
        QTimer::singleShot(200,this,SLOT(slot_Down_Button_Push()));
    }


}

///////////Push & Release///////////////////////
void setting::slot_Mode_Change()
{
        if(mode == RF_MODE_VENISTAR) //temperature
        {
            mode = RF_MODE_CONTINUANCE;
        }
        else if(mode == RF_MODE_CONTINUANCE) //temperature
        {
            mode = RF_MODE_TEMPERATURE;
        }
        else if(mode == RF_MODE_TEMPERATURE)
        {
            mode = RF_MODE_VENISTAR;
        }
}

void setting::Hide()
{
    ui->BackButton->hide();
    ui->colorr_label->hide();
    ui->colorr_label_2->hide();
    ui->Up_button->hide();
    ui->Down_button->hide();
    ui->line->hide();
    ui->ModeButton->hide();
    ui->Modelabel->hide();
    ui->TempLCD->hide();
    ui->Temp_line->hide();
    ui->Temp_symbol->hide();
    ui->Temp_symbol_label->hide();
    ui->Temp_Watt_symbol_label->hide();
    ui->TimeLCD_label->hide();
    ui->TimeMLCD->hide();
    ui->TimeM_line->hide();
    ui->TimeSLCD->hide();
    ui->TimeS_line->hide();
    ui->Time_symbol->hide();
    ui->Time_symbol_label->hide();
    ui->WattLCD->hide();
    ui->Watt_line->hide();
    ui->Watt_symbol->hide();
    ui->Watt_symbol_label->hide();
}
void setting::Show()
{
    ui->BackButton->show();
    ui->colorr_label->show();
    ui->colorr_label_2->show();
    ui->Up_button->show();
    ui->Down_button->show();
    ui->line->show();
    ui->ModeButton->show();
    ui->Modelabel->show();
    ui->TempLCD->show();
    ui->Temp_line->show();
    ui->Temp_symbol->show();
    ui->Temp_Watt_symbol_label->show();
    ui->TimeLCD_label->show();
    ui->TimeMLCD->show();
    ui->TimeM_line->show();
    ui->TimeSLCD->show();
    ui->TimeS_line->show();
    ui->Time_symbol->show();
    ui->Time_symbol_label->show();
    ui->WattLCD->show();
    ui->Watt_line->show();
    ui->Watt_symbol->show();
    ui->Watt_symbol_label->show();
}
void setting::installFilter()
{
    ui->TimeMLCD->installEventFilter(this);
    ui->TimeSLCD->installEventFilter(this);
    ui->WattLCD->installEventFilter(this);
    ui->TempLCD->installEventFilter(this);
    ui->Modelabel->installEventFilter(this);
    ui->BackButton->installEventFilter(this);
    ui->ModeButton->installEventFilter(this);
    ui->Up_button->installEventFilter(this);
    ui->Down_button->installEventFilter(this);
}
bool setting::eventFilter(QObject *target, QEvent *event)
{
    if ((target == ui->TimeMLCD) && (event->type() == QEvent::MouseButtonPress))
    {

        if(mode == RF_MODE_VENISTAR)
        {
            global::Click_state[mode] = Click_TimeMin;
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {
            global::Click_state[mode] = Click_TimeMin;
        }
        else
        {
            global::Click_state[mode] = Click_TimeMin;
        }
    }
    else if ((target == ui->TimeSLCD) && (event->type() == QEvent::MouseButtonPress))
    {

        if(mode == RF_MODE_VENISTAR)
        {
            global::Click_state[mode] = Click_TimeSec;
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {
            global::Click_state[mode] = Click_TimeSec;
        }
        else
        {
            global::Click_state[mode] = Click_TimeSec;
        }
    }
    else if ((target == ui->WattLCD) && (event->type() == QEvent::MouseButtonPress))
    {
        if(mode == RF_MODE_VENISTAR)
        {
            global::Click_state[mode] = Click_Watt;
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {
            global::Click_state[mode] = Click_Watt;
        }
        else
        {
            global::Click_state[mode] = Click_Watt;
        }
    }
    else if ((target == ui->TempLCD) && (event->type() == QEvent::MouseButtonPress))
    {
        if(mode == RF_MODE_VENISTAR)
        {
            global::Click_state[mode] = Click_Temp;
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {
            global::Click_state[mode] = Click_Temp;
        }
        else
        {
            global::Click_state[mode] = Click_Temp;
        }

    }
    else if ((target == ui->Modelabel) && (event->type() == QEvent::MouseButtonPress))
    {
        global::cal = true;
        Subtimer->start();
        emit sig_calibration();
    }
    else if((target == ui->BackButton) && (event->type() == QEvent::MouseButtonPress))
    {
        emit sig_setting();
        emit sig_btnReleased(BTNS::BTN_ENCORDER_SW);
    }
    else if((target == ui->ModeButton) && (event->type() == QEvent::MouseButtonPress))
    {
        emit sig_btnReleased(BTNS::BTN_MODE);
        slot_Mode_Change();
    }
    else if((target == ui->Up_button) && (event->type() == QEvent::MouseButtonPress))
    {
        QTimer::singleShot(200,this,SLOT(slot_Up_Button_Push()));
        if(mode == RF_MODE_VENISTAR)
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[0]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[6]=true;
            }
        }
        else if(mode == RF_MODE_CONTINUANCE)
        {

            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[0]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[6]=true;
            }
        }
        else
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[0]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[2]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[4]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[6]=true;
            }
        }
         emit sig_btnDirReleased(BTNS::BTN_DR_RIGHT);
    }
    else if((target == ui->Down_button) && (event->type() == QEvent::MouseButtonPress))
    {
        QTimer::singleShot(200,this,SLOT(slot_Down_Button_Push()));
        if(mode == RF_MODE_VENISTAR)
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[1]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[3]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[7]=true;
            }

        }
        else if(mode == RF_MODE_CONTINUANCE)
        {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[1]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[3]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[7]=true;
            }

       }
       else
       {
            if(global::Click_state[mode]==Click_TimeMin)
            {
                global::state[1]=true;
            }
            else if(global::Click_state[mode]==Click_TimeSec)
            {
                global::state[3]=true;
            }
            else if(global::Click_state[mode]==Click_Watt)
            {
                global::state[5]=true;
            }
            else if(global::Click_state[mode]==Click_Temp)
            {
                global::state[7]=true;
            }
       }
         emit sig_btnDirReleased(BTNS::BTN_DR_LEFT);

    }
    return QWidget::eventFilter(target, event);
}
