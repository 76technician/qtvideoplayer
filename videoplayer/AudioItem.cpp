#include <AudioItem.h>

AudioPlay::AudioPlay(QObject *parent):QObject(parent)
{
//    m_timer.setInterval(500);//单位毫秒
//    connect(&m_timer, &QTimer::timeout, this, &Producer::handleTimeout);
}

QAudioDevice AudioPlay::audioDevice()const
{
    return m_audioDevice;
}

void AudioPlay::setAudioDevice(QAudioDevice newaudioDevice)
{
    if (m_audioDevice == newaudioDevice)
        return;
    m_audioDevice = newaudioDevice;
    emit audioDeviceChanged();
}

void AudioPlay::setFormat(int sampleRate, int sampleSize, int Channels){
    format.setChannelCount(Channels);
    format.setSampleRate(sampleRate);
    if(sampleSize==8) format.setSampleFormat(QAudioFormat::UInt8);
    else if(sampleSize==16) format.setSampleFormat(QAudioFormat::Int16);
    else  format.setSampleFormat(QAudioFormat::Unknown);
    m_audioDevice=QMediaDevices::defaultAudioOutput();
    if (!m_audioDevice.isFormatSupported(format)) {
            qDebug()<< sampleRate << Channels << sampleSize << QStringLiteral("format is");
           qWarning() << "Raw audio format not supported by backend, cannot play audio.";
           return;
    }
    audio = new QAudioSink(format, this);
    sound_device=audio->start();
    sound_device->open(QIODevice::WriteOnly);
    //qDebug() << QStringLiteral("设置完成");
}

void AudioPlay::play_sound(uchar *data, int len){
    qDebug() << QStringLiteral("play_sound");
    //while(audio->bytesFree()<len);
    int free_len=audio->bytesFree();
    int cur_len=len;
        qDebug() << QStringLiteral("play_sound start")<<cur_len;
    if(len<=0){
        free(data);
        return ;
    }
    while(len!=0){
        if(free_len>=len){
            sound_device->write((char*)data,len);
            data+=len;
            len=0;
        }else{
            qDebug() << QStringLiteral("不够用");
            if(free_len>0){
            sound_device->write((char*)data,free_len);
            data+=free_len;
            len-=free_len;
            }
        }
        free_len=audio->bytesFree();
    }
    data-=cur_len;
    free(data);
    qDebug() << QStringLiteral("play_sound free");
}

