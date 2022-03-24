#include <VideoThread.h>
#include <QImage>
#include <QPainter>
#include <QSize>
#include <QVideoFrame>

#include <QRandomGenerator>
#include <QDateTime>

void VideoThread::seek(long long pts){
    isSeek=true;
    shouldSeek=shouldSeek?false:true;
    seek_pts=pts;
}
void VideoThread::stop(){
    Stop_Quit=true;
}
void VideoThread::open(){
    Stop_Quit=false;
}
void VideoThread::run(){
//    std::lock_guard<std::mutex> guard(video_mutex);
    while(true){
    if((isSeek && shouldSeek) || Stop_Quit){
        int len=cache.size();
        while(cache.size()!=0){
            AVFrame* cur=cache.dequeue();
            len--;
            av_frame_free(&cur);
        }
        len=images.size();
        while(len!=0){
            QImage cur_image=images.dequeue();
            delete[] cur_image.bits();
            len--;
        }
        isSeek=false;
        avcodec_flush_buffers(vCodec);
        msleep(1);
        if(Stop_Quit) break;
       continue;
    }
    while(!cache.isEmpty() && !Stop_Quit){
        if(isSeek && shouldSeek) break;
        AVFrame* cur=cache.dequeue();
        //qt video frame 执行的为双缓冲 应该每两个删除一个 若只删除上一个 会导致窗口移动再绘制时无法找到被移动的那张图片
        //而低帧率 移动窗口不会终止 因为移动时 空间未被delete
        //若帧的照片多于2张 则可以删除
        if(images.size()>10){
           QImage cur_image=images.dequeue();
           if(cur_image.bits())
            delete[] cur_image.bits();
        }
        if(isSeek && shouldSeek) break;
         QImage image = Repaint(cur);
         images.enqueue(image);
          qDebug() << QStringLiteral("it is should show picture");
         emit send_image(image);
          if(flag==0){
              //为0则为本地视频播放
              long long sleep_time=cur->pts*time_base;
              sleep_time=(sleep_time-sync)<0?2:(sleep_time-sync);
              qDebug() << QStringLiteral("show picture sleep");
              if(sleep_time>40) sleep_time=30;
              if(isSeek && shouldSeek){
                  av_frame_free(&cur);
                  break;
              }
              else   msleep((int)sleep_time);
          }
         qDebug() << cache.size() << QStringLiteral("free one frame");
         av_frame_free(&cur);
    }
    msleep(1);
    }

}

void VideoThread::receive_pkt(AVPacket* pkt_a){
      //AVPacket* pkt=pkt_a;
      AVFrame *pAVFrame = av_frame_alloc();
      ret = avcodec_send_packet(vCodec, pkt_a);
      if (ret != 0){
           qDebug() << QStringLiteral("receive pkt from video is failed");
        }
       ret = avcodec_receive_frame(vCodec, pAVFrame);
       if (ret != 0){
           qDebug() << QStringLiteral("receive pAVFrame is failed");
           av_packet_unref(pkt_a);
           av_frame_free(&pAVFrame);
           return ;
        }
       cache.enqueue(pAVFrame);
       qDebug() << cache.size() << QStringLiteral("add one frame");
       av_packet_unref(pkt_a);
}

QImage VideoThread::Repaint(AVFrame *frame)
{
    qDebug() << QStringLiteral("repaint");
    // Convert YUV420P -> RGB32 by libswscale
    int outWidth=std::min(640,frame->width),outHeight=std::min(480,frame->height);
//    if (vctx==NULL)
//    {
        vctx = sws_getContext(frame->width,
                                       frame->height,
                                       static_cast<AVPixelFormat>(frame->format),
                                       outWidth,
                                       outHeight,
                                       AV_PIX_FMT_BGRA,
                                       SWS_FAST_BILINEAR, NULL, NULL, NULL);
//    }
//    int linesize[8] = {frame->linesize[0] * 3};
    int linesize[8] = {outWidth * 4};
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, outWidth, outHeight, 1);
    uint8_t* pRGBBuffer = new uint8_t[num_bytes];
    uint8_t* szRGBBuffer[8] = {pRGBBuffer};
    ret=sws_scale(vctx,
              frame->data,
              frame->linesize,
              0,
              frame->height,
              szRGBBuffer,
              linesize);
    sws_freeContext(vctx);
    return  QImage(pRGBBuffer,
                           outWidth,
                           outHeight,
                           QImage::QImage::Format_ARGB32,
                           [](void *info)->void
                                {
                                    uint8_t* p = static_cast<uint8_t*>(info);
                                    delete[] p;
                                });
}

void VideoThread::video_encode_init(AVCodecParameters *para,long long time_base){
    vCode=avcodec_find_decoder(para->codec_id);//视频解码器
//    videoWidth = para->width;
//    videoHeight = para->height;
    //将视频流信息提出来
    vCodec=avcodec_alloc_context3(vCode);
    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(vCodec, para)) < 0) return ;
    vCodec->thread_count=8;
   //打开解码器
    ret = avcodec_open2(vCodec,vCode,nullptr);
    if(ret < 0){
        qDebug() << QStringLiteral("打开解码器失败");
        return;
    }
    this->time_base=time_base;
}

void VideoThread::setSync(long long mesc){
    this->sync=mesc;
    qDebug() << QStringLiteral("set mesc");
}

double VideoThread::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

void VideoThread::set_mod(long long  cur){
    qDebug() << QStringLiteral("video thread set sync");
    this->flag=cur;
    qDebug() << QStringLiteral("video thread set over");
}
