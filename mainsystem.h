#ifndef MAINSYSTEM_H
#define MAINSYSTEM_H

#include <QWidget>
#include <QObject>
#include "global.h"
#include "sysparm.h"
#include "outputmode.h"
#include "setting.h"
#include "calibration.h"
#include "sysmessage.h"
#include "initializing.h"
#include "audiooutput.h"
#include <QTimer>
#include <QElapsedTimer>

class MonitoringTask;
class OutputThread;
class GPIOButton;

namespace global
{
    extern uint16 TimeM[3];
    extern uint16 TimeS[3];
    extern uint16 Watt[3];
    extern uint16 Temp[3];
    extern uint16 state[8]; //TimeM , TimeS , Watt , Temp
    extern uint16 Push_state[8];
    extern uint16 state_en[11];
    extern uint16 Start;
    extern uint16 qgauge_visible;
    extern uint16 qgauge_enable;
    extern uint16 System_Initializing_visible;
    extern uint16 Click_state[3]; //venistar, continu , temperatuer
    extern bool cal;
    extern QString Display;
}

namespace Ui {
class mainsystem;
}

class mainsystem : public QWidget
{
    Q_OBJECT

public:
    // continuance mode , temperater mode
    uint16 Display_TimeM[2]={0,0};
    uint16 Display_TimeS[2]={0,0};
    uint16 Display_Watt[2]={0,0};
    uint16 Display_Temp[2]={0,0};
    uint16 Setting_CM_Mode = false;
    uint16 Venistar_CM_Mode = false;
    uint16 Temperature_CM_Mode = false;
    uint16 Step[2];
    uint16 i[8]={0,};
    uint16 Enable_flag = 0;
    uint16 BT_flag = 0;
    uint16 Mode_flag = 0;
    QElapsedTimer MessageTimer;
    QElapsedTimer CM_Mode_ElapsedTimer;
    QElapsedTimer Button_ElapsedTimer;


    enum BTNS
    {
        BTN_ELECTRODE_RFSTARTSTOP = 0,
        BTN_RFSTARTSTOP,
        BTN_MODE,
        BTN_OHMECHECK,
        BTN_ENCORDER_SW,
        BTN_MAX,
        BTN_DR_LEFT,
        BTN_DR_RIGHT
    };

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
        STATE_ERROR_LOW_IMPEDANCE,
        STATE_ERROR_NeedTimeSec,
        STATE_LAST

    };
    enum RF_mode
    {
        RF_MODE_VENISTAR,
        RF_MODE_CONTINUANCE,
        RF_MODE_TEMPERATURE
    };

    enum RF_Starstop
    {

        RF_Stop,
        RF_Start
    };
    enum Click_State
    {
        Click_None,
        Click_TimeMin,
        Click_TimeSec,
        Click_Watt,
        Click_Temp
    };
    enum RF_Output_CM_Mode_en
    {
        RF_DEVICE_OP_MODE_3CM = 0,
        RF_DEVICE_OP_MODE_7CM
    };
    bool eventFilter(QObject *target, QEvent *event);
    explicit mainsystem(QWidget *parent = 0);
    virtual ~mainsystem();


    mainsystem::State_en getState() const;
    void setState(const mainsystem::State_en state);

    GPIOButton *pGButton;
    SysParm *sysParm;
    RF_DEVICE_FACTORS *mRfDeviceFactors;
    QTimer diplayTimer;
    QElapsedTimer elapsedTime;
    QElapsedTimer Cutoff_tmElapsedTime;

    void setIDreg(int i) {id_reg = i; }
    void setIDsemi(int i) {id_semi = i; }
    void setIDstarmed(int i) {id_starmed = i; }

private slots:
    void slot_Hide();
    void slot_Show();
    void slot_EnableTime();
    void slot_SysMessaged(mainsystem::State_en);
    void slot_Display();
    void slot_Set_Display();
    void slot_CM_Mode_Clicked();
    void slot_Setting_Enable();
    void slot_RF_Button_Pushed();
private:
    MonitoringTask *monitoringTask;
    OutputThread *outputThread;
    State_en _state;
    QString strDisp;
    QString strTime;
    uint32 data_in_index;
    void RFParameter(bool isRead);
    void installFilter();
    void SystemInitialize();
    void FinishInitialize();
    void DisplaySetParameter(int32 param);
    void StartMenuSetting();
    void Button_Move(mainsystem::RF_mode mode);
    void Enavble_False();
    void Enavble_True();
    void Limited_Button(mainsystem::RF_mode mode);
    void SetFont();
    void LCD_Display(mainsystem::RF_mode mode);
    void Hide_Show_Display();
    void Setting_Display(mainsystem::RF_mode mode);
    void Symbol_image_fix();
    void Modechange();
    void Test_Hide();
    int id_reg;
    int id_semi;
    int id_starmed;
    int32 uartTime;
    setting *Setting_Screen;
    sysmessage *Error_Screen;
    initializing *Initializing_Screen;
    calibration *Calibration_Screen;
    AudioOutput *Audio_Output;
    QTimer *EnableTimer;
    QTimer *DisplayTimer;
    QTimer *UpTimer;
    QTimer *DownTimer;
    Ui::mainsystem *ui;

signals:
    void KeyPressed(char ch);
    void sig_btnReleased(int);
    void sig_btnDirReleased(int);
    void sig_btnLongPressed(int);

    void sig_Sysmessage(mainsystem::State_en);
    void sig_sysmessage(mainsystem::State_en);
    void sig_init();
    void sig_Wattgauge(int);
    void sig_Tempgauge(int);
    void sig_setting_display(mainsystem::RF_mode);
    void sig_calibration();
    //void sig_audiostate(AudioOutput::eAUDIOS);
    void sig_audiostate();
public slots:
    void slot_update();
    void slot_btnReleased(int);
    void slot_btnDirReleased(int);
    void slot_btnLongPressed(int);
    void slot_init_screen();
    void doRun();

    void slot_modeChanged(OutputMode::RF_Output_Mode_en);
    void slot_rfStarted();
    void slot_rfStopped();

    void slot_overTemperatureWarning();
    void slot_overImpedanceWarning();
    void slot_lowImpedanceWarning();

  //  void SysMessage_clicked();
};

#endif // MAINSYSTEM_H
