#ifndef INITIALIZING_H
#define INITIALIZING_H
#include "global.h"
#include <QWidget>
#include <QTimer>
#include "QElapsedTimer"
#include "mrfa_rf_table.h"
class SysParm;
namespace Ui {
class initializing;
}

class initializing : public QWidget
{
    Q_OBJECT

public:
    int32 i = 0;
    SysParm *sysParm;
    QElapsedTimer elapsedtimer;
    QElapsedTimer waitt_elapsedtimer;
    void setIDstarmed(int i) {id_starmed = i; }
    enum State_Ok
    {
        STATE_V_OK,
        STATE_I_OK,
        STATE_T_OK,
        STATE_W_OK,
        STATE_R_OK
    };
    enum State_Fail
    {
        STATE_V_Fail,
        STATE_I_Fail,
        STATE_T_Fail,
        STATE_W_Fail,
        STATE_R_Fail
    };
    explicit initializing(QWidget *parent=nullptr , SysParm *sysParm=nullptr);
    ~initializing();
public slots:

    void Timer_counter();
    void Timer_state();

private:
    Ui::initializing *ui;
    QTimer *timer;
    QTimer *timercount;

    //Thread *thread;
    void SetFont();
    void Dispaly_Hide();
    void Dispaly_Show(int32 i);
    void Dispaly_State_Ok(int16 state);
    void Dispaly_State_Fail(int16 state);
    int id_starmed;
    //Thread *thread;
signals:

    void sig_init();
};

#endif // INITIALIZING_H
