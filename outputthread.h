#ifndef OUTPUTTHREAD_H
#define OUTPUTTHREAD_H

#include <QObject>
#include "workerthread.h"
#include "outputmode.h"
#include <QTime>
#include <QTimer>
#include <QBasicTimer>
#include "monitoringtask.h"
namespace global
{
    extern uint16_t Start;
    extern uint16 TimeM[3];
    extern uint16 TimeS[3];

}
using namespace std;

class EPSystem;
class SysParm;


class OutputThread : public QObject, public WorkerThread
{
    Q_OBJECT
public:
    explicit OutputThread(QObject *parent = nullptr, SysParm *sysParm = nullptr);

    ~OutputThread();

signals:
    void sig_modeChange(OutputMode::RF_Output_Mode_en);
    void sig_rfStart();
    void sig_rfStop();
    void sig_overTemperature();
    void sig_overImpedance();
    void sig_lowImpedance();


public slots:
    void slot_dispCounter();

private:
    OutputMode *outputMode;
#ifndef __FEATURE_VVR12_BOARD__
    OutputMode *outputCMMode;
#endif
    MonitoringTask *monitoringTask;

    vector<OutputMode*> vModes;
#ifndef __FEATURE_VVR12_BOARD__
    //vector<OutputMode*> vCMModes;
#endif
    bool tmRFPowerActiveState;
    void loop();
    clock_t begin;

public:
    enum RF_Output_CM_Mode_en
    {
        RF_DEVICE_OP_MODE_3CM = 0,
        RF_DEVICE_OP_MODE_7CM
    };
    SysParm *sysParm;
    RF_DEVICE_FACTORS *mRfDeviceFactors;
    QBasicTimer btimer;
    QTime rfTimer;

    uint16 Temperature_CM_mode = 0;
    uint16 Venistar_CM_mode = 0;

    void SetOutputMode(OutputMode::RF_Output_Mode_en = OutputMode::RF_DEVICE_OP_MODE_SIZE);
    #ifndef __FEATURE_VVR12_BOARD__
    void SetOutputCMMode(OutputMode::RF_Output_CM_Mode_en = OutputMode::RF_DEVICE_OP_MODE_3CM);
#endif
    OutputMode::RF_Output_Mode_en GetOutputMode();
    #ifndef __FEATURE_VVR12_BOARD__
    OutputMode::RF_Output_CM_Mode_en GetOutputCMMode();
#endif
    const int8* GetOutputModeTitle();
    void RFStart();
    void RFStop();
    bool GetRFPowerActiveState() const;
    void SetRFPowerActiveState(bool state);
    void MRFA_InitRFADeviceState();
    void MRFA_DeInitRFADeviceState();
    //void DACPortOutputInc(U16, bool pass=true);
    //void DACPortOutputDec(U16, bool pass=true);
    //void DACPortOutputSet(U16);

    void CheckImpedanceState();
    void CheckOverTemperatureState();
    void CheckSafetyState();
    void InitThread(SysParm *sysParm);

    //void timerEvent(QTimerEvent *event);


//for test
private:
    int counter;
    double waitTime;
    QTimer *checkpersec;
};
#endif // OUTPUTTHREAD_H
