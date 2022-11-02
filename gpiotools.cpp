
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <QDebug>
#include "gpiotools.h"
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdarg.h>
#include <termio.h>
#include <sys/timeb.h>
#include <signal.h>


//#define FEATURE_ADS8343
#define FEATURE_ADS8341

using namespace std;

namespace gpioTools {

#define DECODE_MODE		0x09
#define INTENSITY_CONTROL	0x0A
#define SCAN_LIMIT			0x0B
#define SHUTDOWN			0X0C
#define FEATURE				0X0E
#define DISPLAY_TEST		0X0F

static const uint32 IOMUXC = 0x020E0000;
static const uint32 IOMUXC_SW_MUX_CTL_PAD_SD4_DATA0 = 0x020E031C;
static const uint32 GPIO_BASE = 0x0209C000;

//GPIO Memory Map
static const uint32 GPIO1_DR = 0x0209C000;
static const uint32 GPIO2_DR = 0x020A0000;
static const uint32 GPIO3_DR = 0x020A4000;
static const uint32 GPIO4_DR = 0x020A8000;
static const uint32 GPIO5_DR = 0x020AC000;
static const uint32 GPIO6_DR = 0x020B0000;


static volatile uint32 *gpio1_address;
static volatile uint32 *gpio2_address;
static volatile uint32 *gpio3_address;
static volatile uint32 *gpio4_address;
static volatile uint32 *gpio5_address;
static volatile uint32 *gpio6_address;

static const uint32 GPIO_OUTPUT = 1;
static const uint32 GPIO_INPUT = 0;

static int32 _spifd[ALL_SPI_DEVICES];
static uint8 _mode[ALL_SPI_DEVICES];
static uint8 _bits_per_word[ALL_SPI_DEVICES];
static uint32 _speed[ALL_SPI_DEVICES];

static uint8 _tx_buf[ALL_SPI_DEVICES][10];
static uint8 _rx_buf[ALL_SPI_DEVICES][10];

static struct spi_ioc_transfer _io_trans[2];

static int32 uart_fd;

const int8 *usb_uart = "/dev/ttymxc2";
const int8 *pump_uart = "/dev/ttymxc4";

#ifdef _ONPC_
#define SETBIT(ADDRESS, BIT)
#define CLEARBIT(ADDRESS, BIT)
#define TESTBIT(ADDRESS, BIT)

#else
#define SETBIT(ADDRESS, BIT) (ADDRESS |= (1<<BIT))
#define CLEARBIT(ADDRESS, BIT) (ADDRESS &= ~(1<<BIT))
#define TESTBIT(ADDRESS, BIT) (ADDRESS & (1<<BIT))
#endif
#define IOMUXC_BASE_ADDR		0x020E0000


volatile unsigned *mapRegAddr(unsigned long baseAddr)
{
    void *regAddrMap = MAP_FAILED;
    int mem_fd = 0;

    if(!mem_fd)
    {
        if((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
        {
            cout << "Cant open /dev/mem" << endl;
            exit(1);
        }
    }

    regAddrMap = mmap(NULL, 32*8,
                      PROT_READ|PROT_WRITE|PROT_EXEC,
                      MAP_SHARED, mem_fd, baseAddr);

    if(regAddrMap == MAP_FAILED)
    {
        cout << "mmap is failed" << endl;
        exit(1);
    }


    if(close(mem_fd) < 0)
    {
        cout << "cant close /dev/mem" << endl;
        exit(1);

    }
    return (volatile unsigned *)regAddrMap;

}
#ifndef _ONPC_
void uart_init()
{

    //unsigned long baudrate = 115200;
    struct termios options;
    char *tx = "STARmed";

    //uart_fd = open("/dev/ttymxc1", O_RDWR | O_NOCTTY);
    uart_fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY);
    fcntl(uart_fd, F_SETFL, 0);

    tcgetattr(uart_fd, &options);
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;

    options.c_lflag &= ~(ICANON | IEXTEN | ISIG | ECHO);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | INPCK | ISTRIP | IXON | BRKINT );

    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    tcsetattr(uart_fd, TCSANOW, &options);
    write(uart_fd, tx, strlen(tx));

}

void uart_close()
{
    close(uart_fd);
}

void uart_printf(char *pcFmt,...)
{
    va_list ap;
    char pbString[256];
    va_start(ap,pcFmt);
    vsprintf(pbString,pcFmt,ap);
    write(uart_fd, pbString, strlen(pbString));
    va_end(ap);
}
#endif

void gpio_init()
{
#ifndef _ONPC_
    gpio1_address = mapRegAddr(GPIO1_DR);
    gpio2_address = mapRegAddr(GPIO2_DR);
    gpio3_address = mapRegAddr(GPIO3_DR);
    gpio4_address = mapRegAddr(GPIO4_DR);
    gpio5_address = mapRegAddr(GPIO5_DR);
    gpio6_address = mapRegAddr(GPIO6_DR);
#endif
    WriteGPIOPortLow(GPIO_3, 20);
}

volatile unsigned *GetEncorderAddress()
{
    return gpio2_address;
}

uint32 ReadGPIOPort(uint32 gpio, uint32 pin)
{
#ifdef _ONPC_
    uint32 result = 1;
    return result;
#else

    switch(gpio)
    {
        case GPIO_1:
            if(TESTBIT(*(gpio1_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_2:
            if(TESTBIT(*(gpio2_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_3:
            if(TESTBIT(*(gpio3_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_4:
            if(TESTBIT(*(gpio4_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_5:
            if(TESTBIT(*(gpio5_address), pin)) return GPIO_HIGH;
            break;

        case GPIO_6:
            if(TESTBIT(*(gpio6_address), pin)) return GPIO_HIGH;
            break;
    }

    return GPIO_LOW;
#endif
}


void WriteGPIOPort(uint32 gpio, uint32 pin, bool flag)
{
    /*
     * 	flag : true is HI, false is LOW
     *
     */
#ifdef _ONPC_
    //qDebug("gpio=%d, pin=%d, flag=%d\n", gpio, pin, flag);
#else
    switch(gpio)
    {
        case GPIO_1:
            if(flag)
                SETBIT(*(gpio1_address), pin);    //HI
            else
                CLEARBIT(*(gpio1_address), pin);    //Low
            break;

        case GPIO_2:
            if(flag)
                SETBIT(*(gpio2_address), pin);    //HI
            else
                CLEARBIT(*(gpio2_address), pin);    //Low
            break;

        case GPIO_3:
            if(flag)
                SETBIT(*(gpio3_address), pin);    //HI
            else
                CLEARBIT(*(gpio3_address), pin);    //Low
            break;
        case GPIO_4:
            if(flag)
                SETBIT(*(gpio4_address), pin);    //HI
            else
                CLEARBIT(*(gpio4_address), pin);    //Low
            break;

        case GPIO_5:
            if(flag)
                SETBIT(*(gpio5_address), pin);    //HI
            else
                CLEARBIT(*(gpio5_address), pin);    //Low
            break;

        case GPIO_6:
            if(flag)
                SETBIT(*(gpio6_address), pin);    //HI
            else
                CLEARBIT(*(gpio6_address), pin);    //Low
            break;

        default:
            break;
    }
#endif
}

void WriteGPIOPortHigh(uint32_t port_id, uint32_t port_value)
{
    WriteGPIOPort(port_id, port_value, GPIO_HIGH);
}

void WriteGPIOPortLow(uint32_t port_id, uint32_t port_value)
{
    WriteGPIOPort(port_id, port_value, GPIO_LOW);
}

void EnableAmplifierPowerOutput()
{
    WriteGPIOPort(GPIO_3, 22, GPIO_HIGH);   //MASTER CLK
    WriteGPIOPort(GPIO_2, 17, GPIO_HIGH);   //AMP_POWER_ON
}

void DisableAmplifierPowerOutput()
{
    WriteGPIOPort(GPIO_2, 17, GPIO_LOW);   //AMP_POWER_ON
    WriteGPIOPort(GPIO_3, 22, GPIO_LOW);   //MASTER CLK
}

void SetElectrodeType(ELECTRODE_Type_en eType)
{
    switch(eType)
    {
        case ELECTRODE_MONOPOLAR:
            WriteGPIOPort(GPIO_3, 31, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 30, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;

        case ELECTRODE_BI_3CM:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;

        case ELECTRODE_BI_7CM:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_HIGH);
            WriteGPIOPort(GPIO_3, 29, GPIO_HIGH);
            break;

        default:
            WriteGPIOPort(GPIO_3, 31, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 30, GPIO_LOW);
            WriteGPIOPort(GPIO_3, 29, GPIO_LOW);
            break;

    }
}

void SetAmplifierPowerOutputDirection(RF_OutputDir_en dir)
{
    if(dir == RFOUTPUT_DIR_TARGET)
    {
        WriteGPIOPort(GPIO_2, 18, GPIO_HIGH);    //RF_IN_DIR
    }
    else
    {
        WriteGPIOPort(GPIO_2, 18, GPIO_LOW);
    }
}

void OnLED(LED_Output_en id)
{
    switch(id)
    {
        case Leds_HANDLE_LED:
            WriteGPIOPort(GPIO_2, 28, GPIO_HIGH);
            break;
  }
}

void OffLED(LED_Output_en id)
{
    switch(id)
    {
        case Leds_HANDLE_LED:
            WriteGPIOPort(GPIO_2, 28, GPIO_LOW);
            break;

    }

}

  void StopSound()
  {
      std::cout << "StopSound" << std::endl;
  }

  void StartSound(uint32_t id)
  {
      std::cout << "StartSound" << std::endl;
  }


void spi_init()
{
    for(int32 i=0; i<ALL_SPI_DEVICES; i++)
    {
        _spifd[i] = -1;
    }

    spiOpen(SPI_DAC7611);
    spiOpen(SPI_ADS8343_A);
    spiOpen(SPI_ADS8343_T);

}

void spiAllclose()
{
    for(int32 i = 0; i<ALL_SPI_DEVICES; i++)
    {
        if(_spifd[i] != -1)
        {
            spiClose(_spifd[i]);
        }
        _spifd[i] = -1;
    }
}



int32 spiOpen(SPI_DEVICES dn)
{
    std::string devspi;

    /*
     *  SPI_CPOL = 0 : low clock ,
     * 							1 : High clock
     *
     * SPI_CPHA = 0 : rising edge
     *                   			1 : falling edge
     *
     *  SPI_MODE_0		(0|0)
     *	 SPI_MODE_1		(0|SPI_CPHA)
     *	 SPI_MODE_2		(SPI_CPOL|0)
     *	 SPI_MODE_3		(SPI_CPOL|SPI_CPHA)
     *
     */

    switch(dn)
    {
       case SPI_DAC7611:
            devspi = "/dev/spidev0.0";
            _mode[SPI_DAC7611] = SPI_MODE_3;
            _bits_per_word[SPI_DAC7611] = 16;
            _speed[SPI_DAC7611] = 2000000;
            break;

        case SPI_ADS8343_A:
            devspi = "/dev/spidev1.0";
            _mode[SPI_ADS8343_A] = SPI_MODE_0;
            _bits_per_word[SPI_ADS8343_A] = 8;
            _speed[SPI_ADS8343_A] = 2000000;
            break;

        case SPI_ADS8343_T:
            devspi = "/dev/spidev1.1";
            _mode[SPI_ADS8343_T] = SPI_MODE_0;
            _bits_per_word[SPI_ADS8343_T] = 8;
            _speed[SPI_ADS8343_T] = 2000000;
            break;

        case SPI_VOL_CTRL_1:
            devspi = "/dev/spidev2.0";
            _mode[SPI_VOL_CTRL_1] = SPI_MODE_0;
            _bits_per_word[SPI_VOL_CTRL_1] = 32;
            _speed[SPI_VOL_CTRL_1] = 2000000;
            break;

        case SPI_VOL_CTRL_2:
            devspi = "/dev/spidev2.1";
            _mode[SPI_VOL_CTRL_2] = SPI_MODE_0;
            _bits_per_word[SPI_VOL_CTRL_2] = 32;
            _speed[SPI_VOL_CTRL_2] = 2000000;
            break;

        default:
            printf(">>can not find spidev\n");
            return -1;
    }

    _spifd[dn] =  spiOpen(devspi.c_str(), &_mode[dn], &_bits_per_word[dn], &_speed[dn]);

    return _spifd[dn];
}

int32 spiOpen(const char *devspi, uint8 *_mode, uint8 *_bits_per_word, uint32 *_speed)
{
	int32 res = 0;
	int32 fd = -1;


#ifdef _ONPC_
	fd = 1;
#else
	fd = open(devspi, O_RDWR);
	if (fd < 0)
	{
		qDebug("cannot open spi device:%s\n",devspi);
		return -1;
	}

	res = ioctl(fd, SPI_IOC_WR_MODE, _mode);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_WR_MODE:0x%x\n", *_mode);
		return -1;
	}

	res = ioctl(fd, SPI_IOC_RD_MODE, _mode);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_RD_MODE:0x%x\n", *_mode);
		return -1;
	}
	/*
	* bits per word
	*/
	res = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, _bits_per_word);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:0x%x\n", *_bits_per_word);
		return -1;
	}

	res = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, _bits_per_word);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:0x%x\n", *_bits_per_word);
		return -1;
	}

	/*
	* max speed hz
	*/
	res = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, _speed);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:%d\n", *_speed);
		return -1;
	}

	res = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, _speed);
	if (res == -1)
	{
		qDebug("can't set spi SPI_IOC_WR_BITS_PER_WORD:%d\n", *_speed);
		return -1;
	}
#endif
	return fd;
}

int32 spiClose(int32 _spifd)
{
    int32 res = -1;

#ifndef _ONPC_
    res = close(_spifd);
    if(res < 0)
    {
        qDebug("error Spi close spifd=0x%x\n", _spifd);
        return -1;
    }
#endif
    return res;
}


int32 ADC_Send(SPI_DEVICES dev, uint8 channel)
{
	//0x9247 / FFFF x 5 volts = 2.86V
	uint16 readValue = 0;
	uint8 controlByte = 0;
	if(dev == SPI_ADS8343_A)
	{
		WriteGPIOPort(GPIO_2, 26, GPIO_LOW);
	}
	else if(dev == SPI_ADS8343_T)
	{
		WriteGPIOPort(GPIO_2, 27, GPIO_LOW);
	}
	else
	{
		return -1;
	}
#ifdef FEATURE_ADS8341
	controlByte = channel; //test(channel & 0x01)<<2 | (1 << (channel>>1));
#else //ADS8344
	controlByte = (channel & 0x01)<<2 | (channel & 0x06)>>1;
#endif
	_tx_buf[dev][0] = 0x87 | (controlByte & 0x07)<<4;
	_tx_buf[dev][1] = 0;
	_tx_buf[dev][2] = 0;
	_tx_buf[dev][3] = 0;
	_rx_buf[dev][0] = 0;
	_rx_buf[dev][1] = 0;
	_rx_buf[dev][2] = 0;
	_rx_buf[dev][3] = 0;

	//length 4 : command(8B) , receive1(8B), receive2(8B), receive3(8B), SPI_IOC_MESSAGE(1)
	spiWriteRead(dev, &_tx_buf[dev][0], &_rx_buf[dev][0], 4);

	WriteGPIOPort(gpioTools::GPIO_2, 27, gpioTools::GPIO_HIGH);
	WriteGPIOPort(gpioTools::GPIO_2, 26, gpioTools::GPIO_HIGH);

#ifdef FEATURE_ADS8341
	//AD8341
	readValue = (_rx_buf[dev][1] & 0x7F) << 9;
	readValue |= (_rx_buf[dev][2] & 0xFF) << 1;
	readValue |= (_rx_buf[dev][3] & 0x80) >> 7;
#else
	//VVR AD8343
	//5V : x = 65534 :
	readValue = (_rx_buf[dev][3] & 0xFF) << 8;
	readValue |= (_rx_buf[dev][2] & 0xFF);
#endif

#if 0 //hw test
    if(dev == SPI_ADS8343_A && channel == 0x01)
	{
		qDebug("cuurent:0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Current = %d",_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
	}

    else if(dev == SPI_ADS8343_A && channel == 0x05)
	{
		qDebug("voltage:0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result AMP_Voltage = %d",_rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
	}

    else if(dev == SPI_ADS8343_T && channel == 0x01)
    {
        qDebug("0x%02x) 0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result Temp A= %d",channel, _rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
    }
	else if(dev == SPI_ADS8343_T && channel == 0x05)
	{
        qDebug("0x%02x) 0:0x%02x, 1:0x%02x, 2:0x%02x, 3:0x%02x, result Temp B= %d",channel, _rx_buf[dev][0], _rx_buf[dev][1], _rx_buf[dev][2], _rx_buf[dev][3], readValue);
	}

#endif
	return readValue;
}

void DAC_Send(uint16 data)
{
	if(data == 0) WriteGPIOPortLow(GPIO_2, 22); //CLR
	else WriteGPIOPortHigh(GPIO_2, 22);         //CLR

	WriteGPIOPort(GPIO_2, 21, GPIO_HIGH);   //LD High
	WriteGPIOPort(GPIO_3, 19, GPIO_LOW);    //CS Low
    	_tx_buf[SPI_DAC7611][0] = 0xff & data ;
    	_tx_buf[SPI_DAC7611][1] = 0x0f & (data>>8);


	//length 2 : command1(8B) , command2(8B), SPI_IOC_MESSAGE(1)	
	spiWrite(SPI_DAC7611, &_tx_buf[SPI_DAC7611][0], 2);

	WriteGPIOPort(GPIO_3, 19, GPIO_HIGH);   //CS High
	WriteGPIOPort(GPIO_2, 21, GPIO_LOW);    //LD Low
	WriteGPIOPort(GPIO_2, 21, GPIO_HIGH);    //LD Low
}


void spiWrite(int32 fdId, uint8 *tbuf, uint32 length)
{
	int32 ret = 0;

	memset(_io_trans, 0, sizeof(_io_trans));

	_io_trans[0].tx_buf = (unsigned long)tbuf;
	_io_trans[0].bits_per_word = _bits_per_word[fdId];
	_io_trans[0].speed_hz = _speed[fdId];
	_io_trans[0].len = length;

#ifndef _ONPC_
	ret = ioctl(_spifd[fdId], SPI_IOC_MESSAGE(1), &_io_trans);
	if(ret == 1)
	{
		printf("Cant send spi message=0x%x\n", _spifd[fdId]);
	}
	else if(ret == -1)
	{
		printf("Failed transferring data=%d\n", errno);
	}
#endif
}

void spiWriteRead(int32 fdId, uint8 *tbuf, uint8 *rbuf, uint32 length)
{
	int32 ret = 0;

	memset(_io_trans, 0, sizeof(_io_trans));

	_io_trans[0].tx_buf = (unsigned long)tbuf;
	_io_trans[0].rx_buf = (unsigned long)rbuf;
	_io_trans[0].bits_per_word = _bits_per_word[fdId];
	_io_trans[0].speed_hz = _speed[fdId];
	_io_trans[0].len = length;


#ifndef _ONPC_
	ret = ioctl(_spifd[fdId], SPI_IOC_MESSAGE(1), &_io_trans);

	if(ret == 1)
	{
	    printf("Cant send spi message=0x%x\n", _spifd[fdId]);
	}
	else if(ret == -1)
	{
	    printf("Failed transferring data=%d\n", errno);
	}
	
#else
#if test
	printf("fdId=%d, spi=%d, bps=%d, speed=%d, len=%d\n",fdId, _spifd[fdId], _bits_per_word[fdId], _speed[fdId], length );
#endif
#endif
	}

}
