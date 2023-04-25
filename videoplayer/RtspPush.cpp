#include "RtspPush.h"

void RtspPush::rtsp_init(){
    avdevice_register_all();
    avformat_network_init();
    ret=avformat_alloc_output_context2(&ofmt_ctx, NULL, "rtsp", rtsp_address);
    if (ret < 0) {
        qDebug() << ret << QStringLiteral("can't open out av format");
        return ;
    }
    ofmt_ctx->max_interleave_delta=40000;// 上下文延迟时间 等待音频流数据 这里设置为40ms

}

void RtspPush::init_out_stream(const AVCodec *codec, AVCodecContext *codectx,int video_or_audio){
        if(!ofmt_ctx) rtsp_init();
        AVStream *stream;
         stream = avformat_new_stream(ofmt_ctx, codec);
        if (!stream){
             qDebug() << "Fail: avformat_new_stream";
            return ;
        }
        stream->id = ofmt_ctx->nb_streams - 1;  //加入到fmt_ctx流
        if(video_or_audio==1){
            stream->time_base.num = 1;
            stream->time_base.den= 25;//16000
            videoindex=stream->id;
        }else{
            stream->time_base.num = 1;
            stream->time_base.den= 44100;
            audioindex=stream->id;
        }
        codectx->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER){
            qDebug() << "set glbal header";
           codectx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
        ret = avcodec_parameters_from_context(stream->codecpar, codectx);
        ofmt_ctx->streams[stream->id]=stream;
        if(ret<0){
            qDebug() << "Fail: avstream copy contextcpr";
            return ;
        }
        qDebug() <<stream->id << "is index";
        //写入两个流再 写入文件头
        if(ofmt_ctx->nb_streams==2){
        qDebug() << "写入文件头";
        AVDictionary* options = NULL;
        av_dict_set(&options, "rtsp_transport", "tcp", 0); //不支持udp
        //av_dict_set(&options, "stimeout", "8000000000000", 0);  //设置超时时间
        //写入头部信息
        ret = avformat_write_header(ofmt_ctx, &options);
        if (ret < 0) {
            qDebug() << ret << QStringLiteral("can't write in header");
            return ;
        }
        }
        //av_dump_format(ofmt_ctx, 0,rtsp_address, 1);
}

void RtspPush::stop(){
    Stop_Quit=true;
    avformat_close_input(&ofmt_ctx);
}
void RtspPush::open(){
    Stop_Quit=false;
}

void RtspPush::run(){
    while(true && Stop_Quit==false){
      //从队列取一个 处理发送
       if(pkts.size()){
           que.lock();
           AVPacket* cur=pkts.dequeue();
           que.unlock();
           //应该为 两个音频帧 一个视频帧率
           ret = av_interleaved_write_frame(ofmt_ctx, cur);
           if (ret < 0){
               qDebug() << ret << QStringLiteral("can't send this pkt");
               continue;
           }else  qDebug() << num << QStringLiteral("had send pkt num is");
           num++;
           pkt_gc.enqueue(cur);
           //保留5s内容 （(44100/1024)+25）*5
           if(pkt_gc.size() >=1000){
                AVPacket* del=pkt_gc.dequeue();
                av_packet_free(&del);
            }
       }
    }
}


void RtspPush::receive_pkt(AVPacket *pkt,int flag){
    qDebug() << pkts.size() << QStringLiteral("cur need send pkts size is");
    qDebug() << pkt->size << QStringLiteral("cur need send pkt size is");
    if(flag) {
        pkt->pts=cur_pts;
        pkt->dts=(pkt->pts);
        cur_pts++;
        pkt->time_base.num=1;
        pkt->time_base.den=25;
//        qDebug() << pkt->pts << QStringLiteral("cur video pts is");
        pkt->stream_index=videoindex;
        que.lock();
        pkts.enqueue(pkt);
        que.unlock();
    } else {
        pkt->stream_index=audioindex;
        pkt->pos=-1;
        pkt->time_base.num=1;
        pkt->time_base.den=44100;
        qDebug() << pkt->pts << QStringLiteral("cur audio pts is");
        pkt->dts=pkt->pts-1;
        if(!video_sync){
        cur_pts=((pkt->pts+800)/1764);
        video_sync=true;
        }
        que.lock();
        pkts.enqueue(pkt);
        que.unlock();
    }
}
