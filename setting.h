#ifndef SETTING_H
#define SETTING_H

#include <QWidget>

#include <QTimer>
#include "sysparm.h"
#include "QElapsedTimer"
#include "calibration.h"
#include "sysmessage.h"

class SysParm;
namespace Ui {
class setting;
}

class setting : public QWidget
{
    Q_OBJECT

public:

   // RF_DEVICE_FACTORS *mRfDeviceFactors;
    QElapsedTimer MessageTimer;
    QElapsedTimer Button_ElapsedTimer;
    uint16 mode;
    enum BTNS
    {
        BTN_ELECTRODE_RFSTARTSTOP = 0,
        BTN_RFSTARTSTOP ,
        BTN_MODE,
        BTN_OHMECHECK,
        BTN_ENCORDER_SW,
        BTN_MAX,
        BTN_DR_LEFT,
        BTN_DR_RIGHT
    };
    enum RF_mode
    {
        RF_MODE_VENISTAR,
        RF_MODE_CONTINUANCE,
        RF_MODE_TEMPERATURE


    };
    enum RF_Starstop
    {
        RF_Start,
        RF_Stop
    };
    enum Click_State
    {

        Click_None,
        Click_TimeMin,
        Click_TimeSec,
        Click_Watt,
        Click_Temp


    };

    bool eventFilter(QObject *target, QEvent *event);
    void setIDstarmed(int i) {id_starmed = i; }
    void Setting_Display(setting::RF_mode mode);
    void Setting_Move(setting::RF_mode mode);
    void Choose_Click_state(setting::RF_mode mode);
    explicit setting(QWidget *parent=nullptr , SysParm *sysParm=nullptr );
    ~setting();
public slots:
    void slot_Mode_Change();
private slots:
    void display();
    void slot_Sub();
    void slot_Up_Button_Push();
    void slot_Down_Button_Push();



public:
    SysParm *sysParm;
private:
    Ui::setting *ui;
    QTimer *Maintimer;
    QTimer *Subtimer;

    QTimer *MinupTimer;
    QTimer *MinDownTimer;
    QTimer *SecUpTimer;
    QTimer *SecDownTimer;
    QTimer *WattUpTimer;
    QTimer *WattDownTimer;
    QTimer *TempUpTimer;
    QTimer *TempDownTimer;
    QTimer *EnableTimer;
    calibration *Calibration_Screen;
    void Show();
    void Hide();
    void Limited_Button(uint16 mode);
    void SetFont();
    int id_starmed;
    bool Button_State;
    void installFilter();
signals:

    void sig_btnReleased(int);
    void sig_btnDirReleased(int);
    void sig_btnLongPressed(int);
    void sig_calibration();
    void sig_setting();
};

#endif // SETTING_H
