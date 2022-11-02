#include "gpiobutton.h"

GPIOButton::GPIOButton(QObject *parent) : QObject(parent)
{
    buttons[ELECTRODE_BTN_RFSTARTSTOP].gpio = gpioTools::GPIO_5;
    buttons[ELECTRODE_BTN_RFSTARTSTOP].pin = 18;
    buttons[ELECTRODE_BTN_RFSTARTSTOP].debounce = 10;
    buttons[ELECTRODE_BTN_RFSTARTSTOP].prevState = BTN_NOT_PRESSED;

    buttons[FOOTSWITCH_BTN_RFSTARTSTOP].gpio = gpioTools::GPIO_5;
    buttons[FOOTSWITCH_BTN_RFSTARTSTOP].pin = 20;
    buttons[FOOTSWITCH_BTN_RFSTARTSTOP].debounce = 10;
    buttons[FOOTSWITCH_BTN_RFSTARTSTOP].prevState = BTN_NOT_PRESSED;


    connect(&timer, SIGNAL(timeout()), this, SLOT(loop()));
    timer.setInterval(10);
    timer.start();
    keyElapsed.start();
}

GPIOButton::~GPIOButton()
{
}

void GPIOButton::readBtnStatus(uint32 id)
{
#ifdef Button_Released
    buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin); //struct
    if(buttons[id].currentState != buttons[id].prevState)
    {
        mdelay(buttons[id].debounce);
        buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin);
        if(buttons[id].currentState == BTN_PRESSED)
        {
            keyElapsed.restart();
            qDebug("id = %d , Pressd" , id);

        }

        else if(buttons[id].currentState == BTN_NOT_PRESSED)
        {
            if((keyElapsed.elapsed() > 100) && !(keyElapsed.elapsed() >= 500))
            {
                emit sig_btnReleased(id);
                qDebug("id = %d , sig_Released" , id);
            }
            else if (keyElapsed.elapsed() >= 500)
            {
                emit sig_btnLongPressed(id);
                qDebug("id = %d , sig_LongPressed" , id);

            }
        }
       buttons[id].prevState = buttons[id].currentState;
    }

#endif
    buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin); //struct
    if(buttons[id].currentState != buttons[id].prevState)
    {
        mdelay(buttons[id].debounce);
        buttons[id].currentState = gpioTools::ReadGPIOPort(buttons[id].gpio, buttons[id].pin);
        if(buttons[id].currentState == BTN_PRESSED)
        {

            keyElapsed.restart();
            Button_state = true;

        }
        else if(buttons[id].currentState == BTN_NOT_PRESSED)
        {
            if((keyElapsed.elapsed() > 100) && !(keyElapsed.elapsed() >= 500))
            {
                emit sig_btnReleased(id);
            }
            Button_state = false;
        }
       buttons[id].prevState = buttons[id].currentState;
    }

    if(Button_state == true)
    {
        if ((keyElapsed.elapsed() >= 500)&&(buttons[id].currentState == BTN_PRESSED))
        {
            emit sig_btnLongPressed(id);
        }
    }
}

void GPIOButton::loop()
{
    for(uint32 i ; i <BTN_MAX ; i ++ )
    {
        readBtnStatus(i);

    }
}
