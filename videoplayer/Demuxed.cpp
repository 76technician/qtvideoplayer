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
            if(videoindex!=-1) break;
        }
     }
    sum=qAfc->duration/AV_TIME_BASE;
     qDebug() << QStringLiteral("demuxed");
     video_decode();
}

void Demuxed::video_decode(){
    qDebug() << QStringLiteral("start video_decode");
    emit video_init(qAfc->streams[videoindex]->codecpar,(long long)
                    (1000 * (r2d(qAfc->streams[videoindex]->time_base))),sum);
//    vCode=avcodec_find_decoder(qAfc->streams[videoindex]->codecpar->codec_id);//视频解码器
//    videoWidth = qAfc->streams[videoindex]->codecpar->width;
//    videoHeight = qAfc->streams[videoindex]->codecpar->height;
//    //将视频流信息提出来
//    vCodec=avcodec_alloc_context3(vCode);
//    /* Copy codec parameters from input stream to output codec context */
//    if ((ret = avcodec_parameters_to_context(vCodec, qAfc->streams[videoindex]->codecpar)) < 0) return ;

//   //打开解码器
//    ret = avcodec_open2(vCodec,vCode,nullptr);
//    if(ret < 0){
//        qDebug() << QStringLiteral("打开解码器失败");
//        return;
//    }
//     qDebug() << QStringLiteral("start video player");
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
    long long audio_pts=0,pts=0;
     bool flag=true;
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
           break;
       }
       qDebug() << seek_pts  << QStringLiteral("seek_pts");
            if(pkt->stream_index==videoindex){
               qDebug() << QStringLiteral("demux video ");
               //以音频帧的dts为准 当前小于则 等待 大于则减少刷新时间
                emit send_pkt(pkt);
                qDebug() << QStringLiteral("demux emited video ");
           }else if(pkt->stream_index==audioindex){
               pts=pkt->pts*(1000 * (r2d(qAfc->streams[audioindex]->time_base)));
               ret = avcodec_receive_frame(aCodec, audioFrame);
               int wait_time=(int)(pts-audio_pts);
                audio_pts=pts;
                //解码播放
                avcodec_send_packet(aCodec, pkt);
                av_packet_unref(pkt);
                ret = avcodec_receive_frame(aCodec, audioFrame);
                if(ret == 0) {
                   channel=audioFrame->channels;
                   if(flag){
                       flag=false;
                       emit format_solt(44100,16,2);
                   }
                   int len=audioFrame->nb_samples*sampleSize*channel/8;
                   uchar* data=(uchar*)malloc(len);
                   len=Resample(audioFrame,data);
                   if(len==0) {
                       free(data);
                       continue;
                    }
                   emit send_pcm(data,len,pts/1000);
                //传递给视频pts
                emit sync(pts);
                 wait_time=wait_time<0?0:wait_time;
                if(wait_time>30) wait_time=20;
                if(isSet && shouldSeek)  msleep(1);
                else  msleep(wait_time);
               }
    }

    }
}

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

void Demuxed::AUDIO_PLAY(){
     AVPacket* pkt = (AVPacket*) malloc(sizeof(AVPacket));
     bool flag=true;
    while (true) {
       if(av_read_frame(qAfc,pkt) <0){
           qDebug() << QStringLiteral("未能获取到pkt") <<"ret is"<<ret;
           break;
       }
         if(pkt->stream_index==audioindex){
                qDebug() << QStringLiteral("音频帧已获取");
                avcodec_send_packet( aCodec, pkt);
                ret = avcodec_receive_frame(aCodec, audioFrame);
                if(ret == 0) {
                   qDebug() << QStringLiteral("生成输入源");
                   //初始化 生成 发送
                   channel=audioFrame->channels;
                   if(flag){
                       flag=false;
                       emit format_solt(44100,16,2);
                   }
                   int duration=(audioFrame->nb_samples*1000)/(sampleRate*audioFrame->channels);
                       qDebug()
                                << audioFrame->nb_samples
                                << sampleRate
                                << audioFrame->channels
                                << duration
                                << QStringLiteral("duration");
                   int len=audioFrame->nb_samples*sampleSize*channel/8;

                   uchar* data=(uchar*)malloc(len);
                   len=Resample(audioFrame,data);
                   long long s=1;
                   emit send_pcm(data,len,s);
                   msleep(duration*channel);
               }
           }
         }
}

//转换为秒为单位的时间基
double Demuxed::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

QImage Demuxed::Repaint(AVFrame *frame)
{

    // Convert YUV420P -> RGB32 by libswscale
    if (nullptr == vctx)
    {
        sws_freeContext(vctx);
        vctx = nullptr;
        vctx = sws_getContext(frame->width,
                                       frame->height,
                                       static_cast<AVPixelFormat>(frame->format),
                                       frame->width,
                                       frame->height,
                                       AV_PIX_FMT_BGRA,
                                       SWS_FAST_BILINEAR, NULL, NULL, NULL);
    }
//    int linesize[8] = {frame->linesize[0] * 3};
    int linesize[8] = {frame->width * 4};
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 1);
    uint8_t* pRGBBuffer = new uint8_t[num_bytes];
    uint8_t* szRGBBuffer[8] = {pRGBBuffer};
    sws_scale(vctx,
              frame->data,
              frame->linesize,
              0,
              frame->height,
              szRGBBuffer,
              linesize);

    return  QImage(pRGBBuffer,
                           frame->width,
                           frame->height,
                           QImage::QImage::Format_ARGB32,
                           [](void *info)->void
                                {
                                    uint8_t* p = static_cast<uint8_t*>(info);
                                    delete[] p;
                                });
}

bool Demuxed::OpenReSample(AVCodecParameters *para)
{
    qDebug() << QStringLiteral("OpenReSample");
    //if (!para)return false;
    //actx分配空间
    actx = swr_alloc_set_opts(NULL,
        av_get_default_channel_layout(2),	//输出格式
        (AVSampleFormat)AV_SAMPLE_FMT_S16,			//输出样本格式 1 AV_SAMPLE_FMT_S16
        para->sample_rate,					//输出采样率
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
    uint8_t *data[2] = { 0 };
    data[0] = d;
    int re = swr_convert(actx,
        data, indata->nb_samples,		//输出
        (const uint8_t**)indata->data, indata->nb_samples	//输入
    );
    int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)AV_SAMPLE_FMT_S16);
    if (re <= 0)return 0;
    return outSize<0?0:outSize;
}

