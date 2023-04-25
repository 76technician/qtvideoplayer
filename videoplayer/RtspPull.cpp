#include "RtspPull.h"

void RtspPull::rtsp_init(){
    avformat_network_init();
    qWarning()<<"rtsp_init";
    AVDictionary* options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0); //udp会花屏
    //av_dict_set(&options, "buffer_size", "1280000", 0);
    if((ret = avformat_open_input(&ifmt_ctx, rtsp_address, NULL, &options)) < 0) {
        qWarning()<<"Cannot open input file"<<ret;
        return ;
    }
    //分析流获得信息
    if((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        qWarning()<<"Cannot find stream information"<<ret;
        return ;
    }
    for(int i=0;i<ifmt_ctx->nb_streams;i++){
        if(ifmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;
        }else if(ifmt_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){
            audioindex=i;
        }
    }
    qWarning()<< audioindex <<"audioindex";
    qWarning()<< videoindex <<"videoindex";
    //初始 采样率 规格 频道 转为 44100 s16 2
    sampleRate = ifmt_ctx->streams[audioindex]->codecpar->sample_rate;//取样率
    qWarning()<< sampleRate <<"sampleRate" << ifmt_ctx->streams[audioindex]->codecpar->format;
    sampleSize=ifmt_ctx->streams[audioindex]->codecpar->bits_per_coded_sample;//位深
    qWarning()<< sampleSize <<"sampleSize";
    emit audio_rtsp_init(ifmt_ctx->streams[audioindex]->codecpar);
    //给h264包写入信息
    bsf = av_bsf_get_by_name("h264_mp4toannexb");
    av_bsf_alloc(bsf, &bsf_ctx);
    avcodec_parameters_copy(bsf_ctx->par_in, ifmt_ctx->streams[videoindex]->codecpar);
    av_bsf_init(bsf_ctx);
    emit video_rtsp_init(ifmt_ctx->streams[videoindex]->codecpar,(long long)
                   (1000 * (r2d(ifmt_ctx->streams[videoindex]->time_base))),ifmt_ctx->duration/AV_TIME_BASE);
}

//转换为秒为单位的时间基
double RtspPull::r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void RtspPull::stop(){
    Stop_Quit=true;
    avformat_close_input(&ifmt_ctx);
}
void RtspPull::open(){
    Stop_Quit=false;
}

void RtspPull::set_nb_samples(int a){
    nb_samples=a;
}

void RtspPull::run(){
    while(true && Stop_Quit==false){
      //从 上下文读取 如果为h264 则使用bsf 更改封装格式
      /*h264有两种封装，一种是annexb模式，传统模式，有startcode（0x000001或0x0000001）分割NALU，
       * 在mpegts流媒体中使用，vlc里打开编码器信息中显示h264;
       *一种是AVCC模式，一般用mp4、mkv、flv容器封装，以长度信息分割NALU, vlc里打开编码器信息中显示avc1。
       *需要使用bsf
       * */
        AVPacket *pkt = av_packet_alloc();
        if(ret = av_read_frame(ifmt_ctx, pkt) < 0){
            qWarning()<<"can't read packet from context"<<ret;
            stop();
            continue;
        }
        if(pkt->stream_index==videoindex){
            //rtp
//            if(ret=av_bsf_send_packet(bsf_ctx, pkt) == 0) {
//                while(ret>=0){
//                AVPacket *s_pkt = av_packet_alloc();
//                ret=av_bsf_receive_packet(bsf_ctx,s_pkt);
//                //发出改好的packet
//                emit rtsp_send_pkt(s_pkt,0);
//                }
//            }
//            av_packet_free(&pkt);
            emit rtsp_send_pkt(pkt,0);
        }else if(pkt->stream_index==audioindex){
            //重采样 发出pcm
//            qWarning()<<pkt->time_base.num << pkt->time_base.den <<"audio sleep time num den";
            emit rtsp_send_pkt(pkt,1);
            wait_time=(nb_samples*1000*1000)/sampleRate;
            wait_time-=1000;
            usleep(wait_time);
            qWarning()<< wait_time
                      <<pkt->size << sampleRate <<"audio pkt size is";
        }
    }
}


