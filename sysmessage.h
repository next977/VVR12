#ifndef SYSMESSAGE_H
#define SYSMESSAGE_H

#include <QDialog>
#include <QPainter>
namespace Ui {
class sysmessage;
}

class sysmessage : public QDialog
{
    Q_OBJECT

public:
    enum SYSMassage {
        HIghImpedance=1,
        Impedanceoutofrange,
        Temperatureover,
        CutoffRfout
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
        STATE_ERROR_None,
        STATE_LAST

    };
    void setIDstarmed(int i) {id_starmed = i; }
    explicit sysmessage(QWidget *parent = 0);
    ~sysmessage();
protected:
    void paintEvent(QPaintEvent *event);

public slots:
   void Timer_state();

private:
    Ui::sysmessage *ui;
    QTimer *timer;
    void SetFont();
    int id_starmed;
};

#endif // SYSMESSAGE_H
