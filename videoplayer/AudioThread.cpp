#include "AudioThread.h"

void AudioThread::stop(){
    Stop_Quit=true;
}
void AudioThread::open(){
    Stop_Quit=false;
}

void AudioThread::run(){
    while(true && Stop_Quit==false){
        audiolock.lock();
        if(!audioque.isEmpty()){
            uchar* cur=audioque.dequeue();
            int cur_len=audiolen.dequeue();
            pts=pts_list.dequeue();
            pts/=sampleRate;
            pts*=44100;
            emit rtsp_sync(pts*1000);
            emit send_pcm(cur,cur_len,0);
        }
        audiolock.unlock();
    }
}

void AudioThread::receive_pkt(AVPacket *pkt){
    timebase=(double)(1/(double)sampleRate);
    qWarning()<<timebase<<sampleRate<<"timebase";
    pts=pkt->pts*timebase;// s为单位
    qWarning()<<pts<<"cur pts";
    ret=avcodec_send_packet(aCodec, pkt);
    if(ret!=0){
        qWarning()<<"decode pkt is failed";
        return ;
    }
    AVFrame *audioFrame = av_frame_alloc();
    ret = avcodec_receive_frame(aCodec, audioFrame);
     qDebug() << QStringLiteral("open send pcm is  2");
     int len=0;
     if(ret == 0) {
        len=audioFrame->nb_samples*32*4/8;
        qDebug() << QStringLiteral("malloc len is ")<<len << audioFrame->channel_layout << audioFrame->nb_samples;
        uchar* data=(uchar*)malloc(len);
        len=Resample(audioFrame,data);
        if(len<=0) {
            qWarning()<<"pcm data size is zero";
            free(data);
        }else{
            audiolock.lock();
            audioque.enqueue(data);
            audiolen.enqueue(len);
            pts_list.enqueue(pts);
            audiolock.unlock();
        }
     }
     av_packet_free(&pkt);
     av_frame_free(&audioFrame);
}

void AudioThread::audio_decode(AVCodecParameters *para){
    qDebug() << QStringLiteral("start audio_decode");
    aCode = avcodec_find_decoder(para->codec_id);
    aCodec = avcodec_alloc_context3(aCode);
    aCodec->thread_count=2;
    if (aCodec == NULL) {
        qDebug() << QStringLiteral("打开解码器上下文失败");
        return;
    }
    qDebug() << QStringLiteral("打开解码器上下文成功");
    // 将新的API中的 codecpar 转成 AVCodecContext
    avcodec_parameters_to_context(aCodec, para);
    qDebug() << QStringLiteral("parameter成功");
    ret = avcodec_open2(aCodec, aCode, NULL);
    if (ret < 0) {
        qDebug() << QStringLiteral("打开解码器失败 ");
        return;
    }
    sampleRate=para->sample_rate;
    emit format_solt(44100,16,2);
    qDebug() << QStringLiteral("start OpenReSample");
    OpenReSample(para);
    qDebug() << QStringLiteral("OpenReSample is down");
}


bool AudioThread::OpenReSample(AVCodecParameters *para)
{
    qDebug() << QStringLiteral("OpenReSample");
    //if (!para)return false;
    //actx分配空间
    actx = swr_alloc_set_opts(NULL,
        av_get_default_channel_layout(2),	//输出格式
        (AVSampleFormat)AV_SAMPLE_FMT_S16,			//输出样本格式 1 AV_SAMPLE_FMT_S16
        44100,					//输出采样率
        av_get_default_channel_layout(para->channels),//输入格式
        (AVSampleFormat)para->format,
        para->sample_rate,
        0, 0
    );
    int re = swr_init(actx);
    if (re != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf) - 1);
        return false;
    }
    return true;
}

//返回重采样后大小 d为转换后的pcm数据
int AudioThread::Resample(AVFrame *indata, uchar *d)
{
    emit set_nb_samples(indata->nb_samples);
    if (!indata) return 0;
    if (!d) return 0;
    uint8_t *data[1] = { 0 };
    data[0] = d;
    int out_counts=av_rescale_rnd(swr_get_delay(actx,indata->sample_rate)+indata->nb_samples,44100,sampleRate,AV_ROUND_UP);
    int re = swr_convert(actx,
        data, out_counts,		//输出
        (const uint8_t**)indata->data, indata->nb_samples	//输入
    );
    int dst_linesize=1,dst_nb_channels=2;
    int outSize =  av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                              re, (AVSampleFormat)AV_SAMPLE_FMT_S16, 1);
    qDebug() << QStringLiteral("outSize is ")<<outSize;
    if (re <= 0)return 0;
    return outSize;
}
