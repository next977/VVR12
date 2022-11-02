#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QFile>
#include <QAudioOutput>
#include <QAudioBuffer>
#include <QAudioProbe>
#include <QTimer>
class AudioOutput : public QObject
{
    Q_OBJECT
public:
    enum eAUDIOS{ SND_RFSTART = 0, SND_RFSTOP };

    explicit AudioOutput(QObject *parent = 0);
    virtual ~AudioOutput();

    void stopSound();
    void infoAudioDevice();
    void infoAudioSource(const QString &name);
    void playSound(AudioOutput::eAUDIOS state);

private:
    QFile sourceFile;
    QAudioOutput* audio;
    QAudioProbe     *probe;
    QTimer *DisplayTimer;
        QTimer *Timer;

signals:

public slots:
    void handleStateChanged(QAudio::State _state);
    void slotProbeBuffer(const QAudioBuffer &buffer);
    void loop_test();
    void looptime_test();
    void start_test();
};

#endif // AUDIOOUTPUT_H
