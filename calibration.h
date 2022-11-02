#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QWidget>
#include <QTimer>
#include "global.h"
#include "gpiotools.h"
#include "sysparm.h"
#include "mrfa_rf_table.h"
namespace Ui {
class calibration;
}


class SysParm;
class calibration : public QWidget
{
    Q_OBJECT

public:
    enum BTNS
    {
        BTN_RFSTARTSTOP = 0,
        BTN_MODE,
        BTN_OHMECHECK,
        BTN_ENCORDER_SW,
        BTN_MAX,
        BTN_DR_LEFT,
        BTN_DR_RIGHT
    };
    enum RF_mode
    {
        RF_MODE_CONTINUANCE,
        RF_MODE_TEMPERATURE

    };
    uint16 is_cal;
    uint16 data_in_index;
    void CalibrationMode();
    void setIDstarmed(int i) {id_starmed = i; }
    bool eventFilter(QObject *target, QEvent *event);
    explicit calibration(QWidget *parent = nullptr, SysParm *sysParm = nullptr);
    ~calibration();
private slots:
    void slot_Up_Button_Push();
    void slot_Down_Button_Push();
    void slot_Main_Display();
    void slot_btnReleasedCal(int keyEvent);
    void slot_btnDirReleasedCal(int keyEvent);
public:
    SysParm *sysParm;
private:
    Ui::calibration *ui;
    QTimer *Maintimer;
    int id_starmed;
    void SetFont();
    void installFilter();

signals:
    void sig_calibration();
    void sig_btnReleased(int);
    void sig_btnDirReleased(int);


};

#endif // CALIBRATION_H
