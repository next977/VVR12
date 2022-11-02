#include "monitoringtask.h"
#include <vector>
#include <QTimer>
#include "mrfa_rf_table.h"
#include "gpiotools.h"

#define __INTERVAL_MONITORING__ 0.01   //30ms, 33 cycle
#define __INTERVAL_TEST__ false
#define DATA_AVERAGE_COUNT 7

#if defined(__GNUC__) || defined(__HP_aCC) || defined(__clang__)
    #define INLINE inline __attribute__((always_inline))
#else
    #define INLINE __forceinline
#endif


using namespace std;

INLINE unsigned int fast_upper_bound2(const vector<uint16>& vec, uint16 value)
{
    unsigned int m_len = vec.size()/2;
    unsigned int low = 0;
    unsigned int high = m_len;

    while (high - low > 1) {
        unsigned int probe = (low + high) / 2;
        unsigned int v = vec[probe];
        if (v > value)
            high = probe;
        else
            low = probe;
    }

    if (high == m_len)
        return m_len;
    else
        return high;
}


MonitoringTask::MonitoringTask(QObject *_parent, SysParm *_sysParm) : QObject(_parent)
{
#if __INTERVAL_TEST__
    QTimer *checkpersec = new QTimer(this);
    counter = 0;
    connect(checkpersec, SIGNAL(timeout()), this, SLOT(slot_dispCounter()));
    checkpersec->start(1000);
#endif
    this->sysParm = _sysParm;
}

MonitoringTask::~MonitoringTask()
{
    if(timer->isActive())
    {
        timer->stop();
    }
    delete timer;
}

void MonitoringTask::RunTimer()
{
 
    timer = new QTimer();
    timer->setInterval(30);
    timer->start();

#if __INTERVAL_TEST__
    connect(timer, SIGNAL(timeout()), this, SLOT(run()));
#else
    connect(timer, SIGNAL(timeout()), this, SLOT(GetAmplifierParameters()));
#endif
 
}

void MonitoringTask::GetAmplifierParameters()
{

    int32 i,j;
    uint32 data_sum;
    uint16 tmp;
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;

    i = 0;
    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x05);   //CH1
        data_sum += MedianFilter(i, 0 , tmp);
    }
    mRfDeviceFactors->tmReadADCVoltage = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);


    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_A, 0x01);   //CH0
        data_sum += MedianFilter(i, 1 , tmp);
    }
    mRfDeviceFactors->tmReadADCCurrent = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);

    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_T, 0x01); //CH0
        //tmp = 30000;
        data_sum += MedianFilter(i, 2 , tmp);
    }
    mRfDeviceFactors->tmReadADCTemperatureA = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);


    data_sum =0;
    for( j = 0; j < DATA_AVERAGE_COUNT; j++)
    {
        tmp = gpioTools::ADC_Send(gpioTools::SPI_ADS8343_T, 0x05); //CH0
        //tmp=58720;
        data_sum += MedianFilter(i, 3 , tmp);
    }
    mRfDeviceFactors->tmReadADCTemperatureB = static_cast<uint16>(data_sum/DATA_AVERAGE_COUNT);



#ifdef __SIMULATOR_AMP__
    for( uint16 k = 0; k < 62; k++ )
    {
        if(mRfDeviceFactors->tmDACValueToSetLast <= sysParm->vRFPowerToDAC.at(k))
        {
            mRfDeviceFactors->tmReadADCCurrent = sysParm->vRFCurrent.at(k);
            mRfDeviceFactors->tmReadADCVoltage = sysParm->vRFVoltage.at(k);
            break;
        }
    }
#endif

    MRFA_TranslateRFTable();
}


int32 MonitoringTask::getVMSfromDAC(uint32 data)
{
    int32 rfa = data;

    if(TransTableRFPowerToDAC[0][0].tmDigitalValue >= rfa)
    {
        return 0;
    }
    else if(TransTableRFPowerToDAC[TABLE_STEP_RFPOWER_REF-1][0].tmDigitalValue<= rfa)
    {
        return TABLE_STEP_VOLTAGE-1;
    }
    else
    {
        int32 bottom = 0;
        int32 top = TABLE_STEP_VOLTAGE - 1;
        int32 mid = 0;

        while(bottom <= top)
        {
            mid = (top+bottom) >> 1;

            if(TransTableRFPowerToDAC[mid][0].tmDigitalValue == rfa)
            {
                return mid;
                break;
            }
            else if(TransTableRFPowerToDAC[mid][0].tmDigitalValue > rfa)
            {
                top = mid - 1;
                if(TransTableRFPowerToDAC[top][0].tmDigitalValue < rfa)
                {
                    return top;
                    break;
                }
            }
            else
            {
                bottom = mid + 1;
                if(TransTableRFPowerToDAC[bottom][0].tmDigitalValue > rfa)
                {
                    return mid;
                    break;
                }
            }
        }
    }
}


void MonitoringTask::MRFA_TranslateRFTable()
{
    uint32 j;

    uint16 rfa,rfb;
    uint16 div_factor;

    uint16 rfCurrentMeasured, rfVoltageMeasured, rfPowerMeasured, rfTemperatureMeasuredA,rfTemperatureMeasuredB;
    RF_DEVICE_FACTORS *mRfDeviceFactors = sysParm->mRfDeviceFactors;

    rfa = mRfDeviceFactors->tmReadADCVoltage;
    j = fast_upper_bound2(sysParm->vRFVoltage, rfa);


    if(j == TABLE_STEP_VOLTAGE)
    {
        rfb = sysParm->vRFVoltage.at(TABLE_STEP_VOLTAGE + j - 1);
    }
    else if(rfa <  sysParm->vRFVoltage.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
    {
        div_factor = (  rfa * 100 ) / (sysParm->vRFVoltage.at(j-1));
        rfb = ( sysParm->vRFVoltage.at(TABLE_STEP_VOLTAGE + j - 1) * div_factor ) /100;
    }
    else
    {
        div_factor = ( ( rfa - sysParm->vRFVoltage.at(j-1) ) * 100 ) /
                     (	sysParm->vRFVoltage.at(j) - sysParm->vRFVoltage.at(j-1) );

        rfb = sysParm->vRFVoltage.at(TABLE_STEP_VOLTAGE + j-1) +
                                 ( ( sysParm->vRFVoltage.at(TABLE_STEP_VOLTAGE + j) - sysParm->vRFVoltage.at(TABLE_STEP_VOLTAGE + j-1))* div_factor ) /100;
    }
    rfVoltageMeasured = rfb;

    //qDebug("voltage = %d, ad = %d\n", mRfDeviceFactors->rfVoltageMeasured, rfa);


    rfa = mRfDeviceFactors->tmReadADCCurrent;
    j = fast_upper_bound2(sysParm->vRFCurrent, rfa);

    if(j == TABLE_STEP_CURRENT)
    {
        rfb = sysParm->vRFCurrent.at(TABLE_STEP_CURRENT + j - 1);
    }
    else if(rfa <  sysParm->vRFCurrent.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
    {
        div_factor = (  rfa * 100 ) / (sysParm->vRFCurrent.at(j-1));
        rfb = ( sysParm->vRFCurrent.at(TABLE_STEP_CURRENT + j-1) * div_factor ) /100;
    }
    else
    {
        div_factor = ( ( rfa - sysParm->vRFCurrent.at(j-1) ) * 100 ) /
                     (	sysParm->vRFCurrent.at(j) - sysParm->vRFCurrent.at(j-1) );

        rfb = sysParm->vRFCurrent.at(TABLE_STEP_CURRENT + j-1) +
                                 ( ( sysParm->vRFCurrent.at(TABLE_STEP_CURRENT + j) - sysParm->vRFCurrent.at(TABLE_STEP_CURRENT + j-1))* div_factor ) /100;
    }
    //if(rfb < 10) rfb = 10;
    rfCurrentMeasured = rfb;

    //qDebug("current = %d, ad = %d\n", mRfDeviceFactors->rfCurrentMeasured, rfa);


    rfa = mRfDeviceFactors->tmReadADCTemperatureA;
    j = fast_upper_bound2(sysParm->vTemperatureA, rfa);

    if(j == TABLE_STEP_TEMPERATURE)
    {
        rfb = sysParm->vTemperatureA.at(TABLE_STEP_TEMPERATURE + j - 1);
    }
    else if(rfa <  sysParm->vTemperatureA.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
    {
        div_factor = (  rfa * 100 ) / (sysParm->vTemperatureA.at(j-1));
        rfb = ( sysParm->vTemperatureA.at(TABLE_STEP_TEMPERATURE + j-1) * div_factor ) /100;
    }
    else
    {
        div_factor = ( ( rfa - sysParm->vTemperatureA.at(j-1) ) * 100 ) /
                     (	sysParm->vTemperatureA.at(j) - sysParm->vTemperatureA.at(j-1) );

        rfb = sysParm->vTemperatureA.at(TABLE_STEP_TEMPERATURE + j-1) +
                                 ( ( sysParm->vTemperatureA.at(TABLE_STEP_TEMPERATURE + j) - sysParm->vTemperatureA.at(TABLE_STEP_TEMPERATURE + j-1))* div_factor ) /100;
    }
    rfTemperatureMeasuredA = rfb;

    rfa = mRfDeviceFactors->tmReadADCTemperatureB;
    j = fast_upper_bound2(sysParm->vTemperatureB, rfa);

    if(j == TABLE_STEP_TEMPERATURE)
    {
        rfb = sysParm->vTemperatureB.at(TABLE_STEP_TEMPERATURE + j - 1);
    }
    else if(rfa <  sysParm->vTemperatureB.at(j-1)) //TransTableRFVoltage[j-1][i].tmDigitalValue )
    {
        div_factor = (  rfa * 100 ) / (sysParm->vTemperatureB.at(j-1));
        rfb = ( sysParm->vTemperatureB.at(TABLE_STEP_TEMPERATURE + j-1) * div_factor ) /100;
    }
    else
    {
        div_factor = ( ( rfa - sysParm->vTemperatureB.at(j-1) ) * 100 ) /
                     (	sysParm->vTemperatureB.at(j) - sysParm->vTemperatureB.at(j-1) );

        rfb = sysParm->vTemperatureB.at(TABLE_STEP_TEMPERATURE + j-1) +
                                 ( ( sysParm->vTemperatureB.at(TABLE_STEP_TEMPERATURE + j) - sysParm->vTemperatureB.at(TABLE_STEP_TEMPERATURE + j-1))* div_factor ) /100;
    }
    rfTemperatureMeasuredB = rfb;

#ifdef __SIMULATOR_AMP__
    if(sysParm->mRfDeviceFactors->tmDACValueToSetLast)
    {
        uint32 index = 0;
        index = getVMSfromDAC(sysParm->mRfDeviceFactors->tmDACValueToSetLast);

        if(index > TABLE_STEP_VOLTAGE)
            index = TABLE_STEP_VOLTAGE;
        else if(index == 0)
            index = 1;

        rfVoltageMeasured = sysParm->mRfDeviceFactors->tmDACValueToSetLast * TransTableRFVoltage[index][0].tmRealValue / TransTableRFPowerToDAC[index][0].tmDigitalValue;
        rfCurrentMeasured = rfVoltageMeasured/60*100;

        //rfPowerMeasured = (rfCurrentMeasured * rfVoltageMeasured) / 100;
        //rfImpedanceMeasured = 60;
    }
#endif



    if(rfCurrentMeasured > 10)
    {
        //mRfDeviceFactors->rfImpedanceMeasured = 100 * mRfDeviceFactors->rfPowerMeasured / (rfCurrentMeasured / 10) / (rfCurrentMeasured / 10);
        rfPowerMeasured = (rfCurrentMeasured * rfVoltageMeasured) / 100;
        mRfDeviceFactors->rfImpedanceMeasured = (rfVoltageMeasured*100)/rfCurrentMeasured;

        if( mRfDeviceFactors->tmDACValueToSetLast == 0)//May be channel interference
        {
            mRfDeviceFactors->rfImpedanceMeasured = 9990;
            rfPowerMeasured = 0;
            rfCurrentMeasured = 0;
        }
    }
    else
    {

        mRfDeviceFactors->rfImpedanceMeasured = 9990;
        rfPowerMeasured = 0;
        rfCurrentMeasured = 0;
    }

    mRfDeviceFactors->rfPowerMeasured = rfPowerMeasured;
    mRfDeviceFactors->rfCurrentMeasured = rfCurrentMeasured;
    mRfDeviceFactors->rfVoltageMeasured = rfVoltageMeasured;
    mRfDeviceFactors->rfTemperatureMeasuredA = rfTemperatureMeasuredA;
    mRfDeviceFactors->rfTemperatureMeasuredB = rfTemperatureMeasuredB;
#if 0
    if( mRfDeviceFactors[i].rfCurrentMeasured > 10 )
                {
                        //----------080804 modified---------//
                    mRfDeviceFactors[i].rfImpedanceMeasured = ((100*(mRfDeviceFactors[i].rfPowerMeasured)/(mRfDeviceFactors[i].rfCurrentMeasured/10))/(mRfDeviceFactors[i].rfCurrentMeasured/10));
                        //----------080804 modified---------//

                //	mRfDeviceFactors[i].rfImpedanceMeasured = ((1000*mRfDeviceFactors[i].rfVoltageMeasured)/(mRfDeviceFactors[i].rfCurrentMeasured));


                    if( !mRfDeviceFactors[i].tmAmplifierActive )
                    {
                        mRfDeviceFactors[i].rfPowerMeasured =  (
                                                       (mRfDeviceFactors[i].rfCurrentMeasured)
                                                       *
                                                       (mRfDeviceFactors[i].rfVoltageMeasured)
                                                       )/100;
                        if( mRfDeviceFactors[i].tmMRFAState != RF_STATE_IMPEDANCE_MONITORING )
                        {
                             mRfDeviceFactors[i].rfImpedanceMeasured = 9990;
                             mRfDeviceFactors[i].rfPowerMeasured = 0;
                        }
                    }else
                    {

                        mRfDeviceFactors[i].rfPowerMeasured =  (
                                                       (mRfDeviceFactors[i].rfCurrentMeasured)
                                                       *
                                                       (mRfDeviceFactors[i].rfVoltageMeasured)
                                                       )/100;


                        /*
                        if( mRfDeviceFactors[i].tmMRFAState == RF_STATE_STOP ) {

                             mRfDeviceFactors[i].rfImpedanceMeasured = 9990;
                             mRfDeviceFactors[i].rfPowerMeasured = 0;
                             mRfDeviceFactors[i].rfCurrentMeasured = 0;


                            //mRfDeviceFactors[i].rfPowerMeasured = 0;
                            //if( i == 1)
                            //CONSOL_Printf("%d,%d\n",  mRfDeviceFactors[i].tmReadADCCurrent,mRfDeviceFactors[i].rfCurrentMeasured );
                        }
                        */
                    }

                    if( mRfDeviceFactors[i].tmDACValueToSetLast == 0)//May be channel interference
                    {
                             mRfDeviceFactors[i].rfImpedanceMeasured = 9990;
                             mRfDeviceFactors[i].rfPowerMeasured = 0;
                             mRfDeviceFactors[i].rfCurrentMeasured = 0;
                    }


                    //if( mRfDeviceFactors[i].rfImpedanceMeasured > 9990 )
                    //		mRfDeviceFactors[i].rfImpedanceMeasured = 9990;
                }
                else
                {
                    mRfDeviceFactors[i].rfImpedanceMeasured = 9990;
                    mRfDeviceFactors[i].rfPowerMeasured = 0;
                }

#endif

}


uint16 MonitoringTask::IOADCSPIPort(int32 n_cs, int32 channel)
{
    uint16 adc_value = 0;

    switch(channel)
    {
        case 2: //voltage
            //adc_value = mRfDeviceFactors->tmReadADCVoltage++;
            if(adc_voltage++ > 60000)
            {
                adc_voltage = 0;
            }
            adc_value = adc_voltage;

            break;

        case 1: //current
            //adc_value = mRfDeviceFactors->tmReadADCCurrent++;
            if(adc_current++ > 40000)
            {
                adc_current = 0;
            }
            adc_value = adc_current;
            break;

        case 0: //temperature
            //adc_value = mRfDeviceFactors->tmReadADCTemperature++;
            if(adc_temp++> 2000)
            {
                adc_temp = 0;
            }
            adc_value = adc_temp;
            break;

    }

#if 0
    //if(channel == 0)
    {
        mRfDeviceFactors->tmReadADCTemperature++;
        if(sysParm->mRfDeviceFactors->tmReadADCTemperature > 60000)
            sysParm->mRfDeviceFactors->tmReadADCTemperature = 0;
        //else if(sysParm->mRfDeviceFactors->tmReadADCVoltage >  55279)
//           sysParm->mRfDeviceFactors->tmReadADCVoltage = 10000;
    }
    return mRfDeviceFactors->tmReadADCTemperature;
#endif
    return adc_value;
}



uint16 MonitoringTask::MedianFilter(int32 deviceid, int32 index, uint16 data)
{
    uint16 i,j,tmp;
    uint16 *p_sort, *p_data;


    switch(index)
    {
        case 0:
            p_data = static_cast<uint16*>(tm_median_voltage[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_voltage_sort[deviceid]);
            break;

        case 1:
            p_data = static_cast<uint16*>(tm_median_current[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_current_sort[deviceid]);
            break;

        case 2:
            p_data = static_cast<uint16*>(tm_median_tempA[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_temp_sortA[deviceid]);
            break;

        case 3:
            p_data = static_cast<uint16*>(tm_median_tempB[deviceid]);
            p_sort = static_cast<uint16*>(tm_median_temp_sortB[deviceid]);
            break;

    }

    //Filtering
    p_sort[0] = p_data[0]=   p_data[1];
    p_sort[1] = p_data[1]=   p_data[2];
    p_sort[2] = p_data[2]=   data;
    //sort
    for( i =0; i < MEDIAN_ORDER - 1; i++)
    {
        for( j =i + 1; j < MEDIAN_ORDER; j++)
        {
            if( p_sort[i] < p_sort[j] )
            {
                tmp = p_sort[j];
                p_sort[j] = p_sort[i];
                p_sort[i] =tmp ;
            }
        }
    }

    //Get Median
    return p_sort[1];

}



void MonitoringTask::slot_dispCounter()
{
    qDebug("MonitoringTask run %d per 1seconds", counter);
    counter = 0;
}


void MonitoringTask::run()
{

    /*
     *  Start call function.
     *  Synchronous call function 일반적인 처리는 동기로
     *  Sensing adc
     */

    GetAmplifierParameters();

    /*
     *  End call function.
     */


#if __INTERVAL_TEST__
    counter++;
#endif

}
