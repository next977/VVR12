#include "audiooutput.h"
#include <QCoreApplication>
#include <stdio.h>
#include <QAudioFormat>
#include <QDebug>
#include <QtEndian>
#include <QBuffer>
#include <QAudioOutput>
#include <QFile>
#include "sounds.h"
AudioOutput::AudioOutput(QObject *parent) : QObject(parent)
{
/*
   // infoAudioDevice();
   // infoAudioSource(":/sound/96Khz_mono.wav");

    QAudioFormat format;
    format.setSampleRate(96000); //96000
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioOutput(format); //audio = new QAudioOutput(format, this);
    DisplayTimer = new QTimer(this);

    probe = new QAudioProbe();
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this,
            SLOT(slotProbeBuffer(QAudioBuffer)));

 //   probe->setSource(audio);

    connect(DisplayTimer, SIGNAL(timeout()), this, SLOT(loop_test())); //Timer
    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    DisplayTimer->start(100);
    //audio->setBufferSize(4027806);
    qDebug("audio->bufferSize() = %d",audio->bufferSize());
*/
    QAudioFormat desiredFormat1;
    desiredFormat1.setChannelCount(1);
    desiredFormat1.setByteOrder(QAudioFormat::LittleEndian);
    desiredFormat1.setCodec("audio/pcm");
    desiredFormat1.setSampleType(QAudioFormat::SignedInt);
    desiredFormat1.setSampleRate(96000);
    desiredFormat1.setSampleSize(16);

    QAudioDeviceInfo info1(QAudioDeviceInfo::defaultOutputDevice());
    if (!info1.isFormatSupported(desiredFormat1))
    {
           qWarning() << "Default format not supported, trying to use the nearest.";
           desiredFormat1 = info1.preferredFormat();
    }

    audio = new QAudioOutput(desiredFormat1);

    DisplayTimer = new QTimer(this);
    Timer = new QTimer(this);


    connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));
    connect(DisplayTimer, SIGNAL(timeout()), this, SLOT(loop_test())); //Timer
    DisplayTimer->start(5000);
    connect(Timer, SIGNAL(timeout()), this, SLOT(looptime_test())); //Timer
   // Timer->start(20);
    qDebug("audio->bufferSize() = %d",audio->bufferSize());

   // playSound(AudioOutput::SND_RFSTART);

}

AudioOutput::~AudioOutput()
{
    if(audio)delete audio;
}
void AudioOutput::slotProbeBuffer(const QAudioBuffer &buffer)
{
    QByteArray byteArray;
    byteArray.append((char *)buffer.data());
    QBuffer mbuffer(&byteArray);

    mbuffer.open(QIODevice::WriteOnly);
    audio->start(&mbuffer);

    mbuffer.close();
}

void AudioOutput::looptime_test()
{
    qDebug("Audio_Timer_Test");
}
void AudioOutput::loop_test()
{
    qDebug("Audio_Loop_Test");
   // audio->stop();
    playSound(AudioOutput::SND_RFSTART);
   // QTimer::singleShot(35 * 1000, this, SLOT(start_test()));

}
void AudioOutput::start_test()
{
    qDebug("Audio_Start!!");
    playSound(AudioOutput::SND_RFSTOP);
}


void AudioOutput::infoAudioDevice()
{

    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        qDebug() << deviceInfo.deviceName();
        QAudioFormat format = deviceInfo.preferredFormat();
        qDebug() << format.sampleRate();
    }
}

void AudioOutput::infoAudioSource(const QString &name)
{
    char strm[4];
    quint32 conv;
    char s[1];
    QByteArray wav;

    QFile infoFile;
    infoFile.setFileName(name);
    if(infoFile.exists())

    infoFile.open(QIODevice::ReadOnly);
    qDebug()<< "RIFF:" << infoFile.read(4);//RIFF

    infoFile.read(strm,4);//chunk size
    qDebug()<< "chunk size:" << qFromLittleEndian<quint32>((uchar*)strm);

    infoFile.read(strm,4);//format
    qDebug()<< "format:" << strm;


    infoFile.read(strm,4);//subchunk1 id
    qDebug()<< "subchunk1 id:" << strm;

    infoFile.read(strm,4);//subchunk1 size
    qDebug()<< "subchunk1 size:" << qFromLittleEndian<quint32>((uchar*)strm);

    infoFile.read(strm,2);//audio format
    qDebug()<< "audio format:" << qFromLittleEndian<quint32>((uchar*)strm);

    infoFile.read(strm,2);//NumChannels
    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<< "NumChannels:" << conv;

    infoFile.read(strm,4);//Sample rate
    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<< "Sample rate:" << conv;
    infoFile.read(strm,4);//Byte rate  : SampleRate * NumChannels * BitsPerSample / 8
    qDebug()<< "Byte rate:" << qFromLittleEndian<quint32>((uchar*)strm);

    infoFile.read(strm,2);//Block Allign
    strm[2] = 0;
    strm[3] = 0;
    qDebug()<< "Block Allign:" << qFromLittleEndian<quint32>((uchar*)strm);

    infoFile.read(strm,2);//BPS
    strm[2] = 0;
    strm[3] = 0;

    conv = qFromLittleEndian<quint32>((uchar*)strm);
    qDebug()<< "BPS:" << conv;


    infoFile.read(strm,4);//subchunk2 id
    qDebug()<< "subchunk2 id:" << strm;

    infoFile.read(strm,4);//subchunk2 size
    qDebug()<< "subchunk2 size:" << qFromLittleEndian<quint32>((uchar*)strm);

/*
    outBuffLen[waveNum] = 0;
    while(!m_WAVFile.atEnd())
    {
        m_WAVFile.read(s,1);
        wav.append(s[0]);
    }
    m_WAVFile.close();
    dsBuffer[waveNum] = QAudioBuffer(wav,fmt);
    outBuffLen[waveNum] = dsBuffer[waveNum].sampleCount();
    qDebug()<<" Processed:";
    qDebug()<<outBuffLen[waveNum];
    wavePath[waveNum] = wavFile;
    QFileInfo fileInfo(wavFile);
    waveName[waveNum] = fileInfo.fileName();
*/
}

void AudioOutput::handleStateChanged(QAudio::State _state)
{
    switch (_state) {
        case QAudio::IdleState:
            // Finished playing (no more data)
            audio->stop();
            sourceFile.close();
            delete audio;
            break;

        case QAudio::StoppedState:
            // Stopped for other reasons
            if (audio->error() != QAudio::NoError) {
                // Error handling
            }
            break;

        default:
            // ... other cases as appropriate
            break;
    }
}

void AudioOutput::playSound(eAUDIOS n)
{
    //QFile sourceFile;
    QBuffer m_buffer;
    QByteArray m_data;

    switch(n)
    {
        case SND_RFSTART:
            m_data = QByteArray::fromRawData(wavname, arraysz);

            //sourceFile.setFileName(":/sound/96Khz_mono.wav");

            break;

        case SND_RFSTOP:
            //sourceFile.setFileName(":/sound/96Khz_mono.wav");
            //sourceFile.setFileName(":/sound/Sample_i_96kHz_30sec.wav");
            break;
    }
/*
    bool p = sourceFile.open(QIODevice::ReadOnly);
    if (p == false)
        qDebug() << "no file";
    else
        qDebug() << "yes file";

*/
    m_buffer.setBuffer(&m_data);
    m_buffer.open(QIODevice::ReadOnly);
    QAudioDeviceInfo d1;
    QList<QAudioDeviceInfo> l1 = d1.availableDevices(QAudio::AudioOutput);

    qDebug() << "======================================================";
    qDebug() << l1.first().supportedCodecs();
    qDebug() << l1.first().supportedChannelCounts();
    qDebug() << l1.first().supportedSampleTypes();
    qDebug() << l1.first().supportedSampleRates();
    qDebug() << l1.first().supportedSampleSizes();


    audio->setVolume(1.0);

    //audio->start(&sourceFile);
    audio->start(&m_buffer);
    qDebug() << "bbbbbbbbbb";

    QEventLoop loop;
    QObject::connect(audio, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
    do {
        loop.exec();
    } while(audio->state() == QAudio::ActiveState);

    qDebug("audio->state() = %d",audio->state());
}

void AudioOutput::stopSound()
{
    audio->stop();
}
