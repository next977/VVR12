#ifndef GPIOTOOLS_H
#define GPIOTOOLS_H

#include "global.h"
#include <QThread>

#define mdelay(n) QThread::msleep(n)
#define udelay(n) QThread::usleep(n)

namespace gpioTools {


    enum SPI_DEVICES {SPI_DAC7611 = 0, SPI_ADS8343_A, SPI_ADS8343_T, SPI_VOL_CTRL_1, SPI_VOL_CTRL_2, ALL_SPI_DEVICES};
    enum SEND_OLED_TYPE{OLED_CMD, OLED_DATA};

    enum en_GPIOPort
    {
        GPIO_1,
        GPIO_2,
        GPIO_3,
        GPIO_4,
        GPIO_5,
        GPIO_6,
        GHIO_LAST
    };

    enum ELECTRODE_Type_en
    {
        ELECTRODE_NONE = 0,
        ELECTRODE_MONOPOLAR,
        ELECTRODE_BI_3CM,
        ELECTRODE_BI_7CM
    };

    enum RF_OutputDir_en
    {
        RFOUTPUT_DIR_DUMMY,
        RFOUTPUT_DIR_TARGET
    };


    enum LED_Output_en
    {
        Leds_HANDLE_LED,
    };

    #define DACPortOutput(a) DAC_Send(a)

    static const uint32 GPIO_HIGH = 1;
    static const uint32 GPIO_LOW = 0;

#ifndef _ONPC_
    void uart_init();
    void uart_printf(char *pcFmt,...);
    void uart_close();
#endif
    void gpio_init();
    volatile unsigned *GetEncorderAddress();
    void WriteGPIOPort(uint32 gpio, uint32 pin, bool flag);
    void WriteGPIOPortHigh(uint32 port_id, uint32 port_value);
    void WriteGPIOPortLow(uint32 port_id, uint32 port_value);



    uint32 ReadGPIOPort(uint32 gpio, uint32 pin);
    void EnableAmplifierPowerOutput();
    void DisableAmplifierPowerOutput();

    void SetElectrodeType(ELECTRODE_Type_en eType);
    void SetAmplifierPowerOutputDirection(RF_OutputDir_en dir);
    void OnLED(LED_Output_en id);
    void OffLED(LED_Output_en id);

    void StopSound();
    void StartSound(uint32 id);

    void spi_init();
    void spiAllclose();

    int spiOpen(SPI_DEVICES dn);
    int spiOpen(const int8 *devspi, uint8 *_mode, uint8 *_bits_per_word, uint32 *_speed);
    void spiWrite(int32 fdId, uint8 *tbuf, uint32 length);
    void spiWriteRead(int32 fdId, uint8 *tbuf, uint8 *rbuf, uint32 length);
    int spiClose(int32 _spifd);


    int32 ADC_Send(SPI_DEVICES dev, uint8 channel);
    void DAC_Send(uint16 data);



}
#endif // GPIOTOOLS_H
