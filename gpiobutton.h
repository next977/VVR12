#ifndef GPIOBUTTON_H
#define GPIOBUTTON_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "gpiotools.h"

class GPIOButton : public QObject
{
    Q_OBJECT

    typedef struct BUTTONS {
        uint32 gpio;
        uint32 pin;
        unsigned long debounce;
        uint32 prevState;
        uint32 currentState;
        bool islongPress;
    }BUTTON;


public:
    explicit GPIOButton(QObject *parent = 0);
    ~GPIOButton();
    enum BTNS{ELECTRODE_BTN_RFSTARTSTOP=0, FOOTSWITCH_BTN_RFSTARTSTOP, BTN_MAX};

    const uint32 BTN_NOT_PRESSED = gpioTools::GPIO_HIGH;
    const uint32 BTN_PRESSED = gpioTools::GPIO_LOW;

    void readBtnStatus(uint32 id);

private:
    QTimer timer;
    BUTTON buttons[BTN_MAX];
    QElapsedTimer keyElapsed;
    bool Button_state = false;
signals:
    void sig_btnReleased(int);
    void sig_btnLongPressed(int);
public slots:
    void loop();
};

#endif // GPIOBUTTON_H
