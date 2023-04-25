#include <Demuxed.h>
void Demuxed::format_init(QUrl path){
    //只能为英文目录
     qDebug() << path << QStringLiteral("qurl 获取到的file path");
   std::string fileurl=path.toEncoded().toStdString();
   fileurl=fileurl.substr(8,fileurl.size());
   this->filePath=fileurl;
   qDebug() <<  QString::fromStdString(fileurl) << QStringLiteral("qurl 获取到的file path");
   this->qAfc=avformat_alloc_context();
}

void Demuxed::format_init(QString path){
     qDebug() << path << QStringLiteral("qstring 获取到的file path");
   this->filePath=path.toStdString();
    qDebug() << path << QStringLiteral("获取到的file path");
   this->qAfc=avformat_alloc_context();
}

void Demuxed::demuxed(){
    ret = avformat_open_input(&qAfc, filePath.c_str(), NULL, NULL);
    qDebug() << QStringLiteral("开始demuxed");
    if (ret < 0)  //如果打开媒体文件失败，打印失败原因
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf) - 1);
        printf("open %s failed:%s\n", filePath.c_str(), buf);
        return ;
    }

    ret = avformat_find_stream_info(qAfc, NULL);
    if (ret < 0)  //如果打开媒体文件失败，打印失败原因
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf) - 1);
        printf("avformat_find_stream_info %s failed:%s\n", filePath.c_str(), buf);
        return ;
    }
    for(unsigned int i = 0; i < qAfc->nb_streams; i++){
        if(qAfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            if(audioindex!=-1) break;
        }else if(qAfc->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioindex = i;
            sampleRate = qAfc->streams[i]->codecpar->sample_rate;//取样率
            sampleSize=qAfc->streams[i]->codecpar->bits_per_coded_sample;//位深
            channel=qAfc->streams[i]->codecpar->channels;
            if(videoindex!=-1) break;
        }
     }
    if(flag){
        flag=false;
        emit format_solt(44100,16,2);
    }
    sum=qAfc->duration/AV_TIME_BASE;
     qDebug() << QStringLiteral("demuxed");
     video_decode();
}

void Demuxed::video_decode(){
    qDebug() << QStringLiteral("start video_decode");
    emit video_init(qAfc->streams[videoindex]->codecpar,(long long)
                    (1000 * (r2d(qAfc->streams[videoindex]->time_base))),sum);
}

void Demuxed::stop(){
    Stop_Quit=true;
    avformat_close_input(&qAfc);
    avcodec_close(aCodec);
}
void Demuxed::open(){
    Stop_Quit=false;
}
void Demuxed::change_process(long long recv_pts){
     isSet=true;
     seek_pts=recv_pts;
     qDebug()<<recv_pts << QStringLiteral("seek pts 1");
     qDebug()<<seek_pts << QStringLiteral("seek pts 2");
     qDebug() << QStringLiteral("recv_pts is ok");
     shouldSeek=shouldSeek?false:true;
}

void Demuxed::run(){
    qDebug() << QStringLiteral("start all run");
    while (true && !Stop_Quit) {
       if(isSet && shouldSeek){
           //两种方式 不用seek 使用pts dts 会导致视频快进慢一些 因为遇到不是关键帧需要解码
           //使用seek 会有一定的错位
           seek_pts*=1000;
           int64_t time_stamp=(int64_t)(seek_pts/(1000 * (r2d(qAfc->streams[videoindex]->time_base))));
           //qDebug()<< time_stamp << seek_pts << QStringLiteral("seek pts 3.5");
           ret=av_seek_frame(qAfc,videoindex,time_stamp,AVSEEK_FLAG_BACKWARD);
           if(ret<0){
               qDebug() << QStringLiteral("av seek is wrong");
           }
           isSet=false;
           continue;
       }
       AVPacket* pkt = (AVPacket*) malloc(sizeof(AVPacket));
       if(av_read_frame(qAfc,pkt) <0){
           qDebug() << QStringLiteral("未能获取到pkt") <<"ret is"<<ret;
            av_packet_free(&pkt);
            stop();
            continue;
       }
       qDebug() << seek_pts  << QStringLiteral("seek_pts");
            if(pkt->stream_index==videoindex){
                qDebug() << QStringLiteral("video num is ")<< qAfc->streams[videoindex]->time_base.num; //0
                qDebug() << QStringLiteral("video den is ")<< qAfc->streams[videoindex]->time_base.den; //1
                qDebug() << QStringLiteral("video pkt num is ")<<pkt->time_base.num;
                qDebug() << QStringLiteral("video pkt den is ")<<pkt->time_base.den;
               qDebug() << QStringLiteral("demux video ");
               //以音频帧的dts为准 当前小于则 等待 大于则减少刷新时间
                emit send_pkt(pkt);
                qDebug() << QStringLiteral("demux emited video ");
           }else if(pkt->stream_index==audioindex){
               pts=pkt->pts* (r2d(qAfc->streams[audioindex]->time_base));// s为单位
               pts/=sampleRate;
               pts*=44100;
               qDebug() << QStringLiteral("audio num is ")<< qAfc->streams[audioindex]->time_base.num;
               qDebug() << QStringLiteral("audio den is ")<< qAfc->streams[audioindex]->time_base.den;
               qDebug() << QStringLiteral("audio pts is ")<<pkt->pts;
                avcodec_send_packet(aCodec, pkt);
               ret = avcodec_receive_frame(aCodec, audioFrame);
                wait_time=(pts-audio_pts);
                audio_pts=pts;
                //解码播放
                //qDebug() << QStringLiteral("wait_time is ")<< wait_time;
                av_packet_unref(pkt);
                //传递给视频pts
                emit sync(pts*1000);
                if(ret == 0) {
                   channel=audioFrame->channels;
                   int len=audioFrame->nb_samples*sampleSize*channel/8;
                   //qDebug() << QStringLiteral("sampleRate is ")<< ;
                   uchar* data=(uchar*)malloc(len);
                   len=Resample(audioFrame,data);
                   if(len==0) {
                       free(data);
                       continue;
                    }
                   emit send_pcm(data,len,pts);
                    qDebug() << QStringLiteral("wait_time is ")<< (unsigned long)wait_time;
                 wait_time=wait_time<0?1:wait_time;
                 wait_time*=1000;
                if(wait_time>30) wait_time=20;
                wait_time*=1000;
                if(isSet && shouldSeek)  msleep(1);
                else  usleep(wait_time);
                qDebug() << QStringLiteral("wait_time is ")<< (unsigned long)wait_time;
               }
    }

    }
}

//qml 调用
void Demuxed::audio_decode(){
    qDebug() << QStringLiteral("start audio_decode");
    aCode = avcodec_find_decoder(qAfc->streams[audioindex]->codecpar->codec_id);
    qDebug() << qAfc->streams[audioindex]->codecpar->codec_id <<QStringLiteral("打开解码器上下文成功");
    aCodec = avcodec_alloc_context3(aCode);
    aCodec->thread_count=8;
    if (aCodec == NULL) {
        qDebug() << QStringLiteral("打开解码器上下文失败");
        return;
    }
    qDebug() << QStringLiteral("打开解码器上下文成功");
    // 将新的API中的 codecpar 转成 AVCodecContext
    avcodec_parameters_to_context(aCodec, qAfc->streams[audioindex]->codecpar);
    qDebug() << QStringLiteral("parameter成功");
    ret = avcodec_open2(aCodec, aCode, NULL);
    if (ret < 0) {
        qDebug() << QStringLiteral("打开解码器失败 ");
        return;
    }
    //emit format_solt(sampleRate,sampleSize,channel);
    qDebug() << QStringLiteral("start OpenReSample");
    OpenReSample(qAfc->streams[audioindex]->codecpar);
    qDebug() << QStringLiteral("OpenReSample is down");
}


//转换为秒为单位的时间基
double Demuxed::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}


bool Demuxed::OpenReSample(AVCodecParameters *para)
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

//返回重采样后大小
int Demuxed::Resample(AVFrame *indata, uchar *d)
{
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

