#include "CaptureAudio.h"
void CaptureAudio::find_audio_device(){
    avdevice_register_all();
    pFormatCtx = avformat_alloc_context();

    ifmt=av_find_input_format("dshow");
    AVDeviceInfoList* list;
    int devices_num=avdevice_list_input_sources(ifmt,"dshow",NULL,&list);
    QString cur_video;
    for(int i=0;i<devices_num;i++){
        if(*(list->devices[i]->media_types)==(AVMediaType::AVMEDIA_TYPE_AUDIO)){
        cur_video=list->devices[i]->device_description;
        emit add_device(cur_video);
        }
    }

    avdevice_free_list_devices(&list);
}
double CaptureAudio::r2d(AVRational r){
     return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void CaptureAudio::stop(){
    Stop_Quit=true;
    avformat_close_input(&pFormatCtx);
    avcodec_close(pCodecCtx);
}
void CaptureAudio::open(){
    Stop_Quit=false;
     //outfilename=std::fopen("../curaudio.aac","wb");
}
void CaptureAudio::audio_codec(QString s){
    //打开设备
    char audio_name[6+s.size()]="audio=";
    int index=6;
    std::string cur=s.toStdString();
    for(auto c:cur) audio_name[index++]=c;
    AVDictionary* options = NULL;
    //设置采集规格
    av_dict_set(&options, "sample_size", "16", 0);
    av_dict_set(&options, "channels", "2", 0);
    av_dict_set(&options, "sample_rate", "44100", 0);
    av_dict_set(&options, "audio_buffer_size", "40", 0);
    //emit format_solt(44100,16,2);
    if(avformat_open_input(&pFormatCtx,audio_name,ifmt,&options)!=0){
        qWarning()<< "Couldn't open audio input stream.";
        return ;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        qWarning()<<"Couldn't find stream information.";
        return ;
    }
    audio_codec();
}
void CaptureAudio::audio_codec(){
    audioindex=-1;
    uint i;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            audioindex=i;
            break;
        }
    if(audioindex==-1)
    {
        qWarning()<<"Couldn't find a video stream.";
        return ;
    }
    pCodec=avcodec_find_decoder(pFormatCtx->streams[audioindex]->codecpar->codec_id);
    if(pCodec==NULL)
    {
        qWarning()<<"Codec not found.";
        return ;
    }
    pCodecCtx=avcodec_alloc_context3(pCodec);
    if ( avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audioindex]->codecpar) < 0) {
        qWarning()<<"can't convert parameters to context.";
        return ;
     }
    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    {
        qWarning()<<"Could not open pCodecCtx.";
        return ;
    }
}
void CaptureAudio::encode_init_context(){
       fifo=av_audio_fifo_alloc(AV_SAMPLE_FMT_S16,2,1);
       eframe=av_frame_alloc();
       eframe->nb_samples=AAC_FRMATE_SIZE;
       eframe->channels=2;
       eframe->sample_rate=44100;
       eframe->format=AV_SAMPLE_FMT_S16;
       eframe->channel_layout=av_get_default_channel_layout(2);
       av_frame_get_buffer(eframe,0);
       //OpenResample();
       //eCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
       eCodec = avcodec_find_encoder_by_name("libfdk_aac");
       if (!eCodec) {
           qWarning()<< "Couldn't init acc codec.";
           return ;
       }
       eCodecCtx = avcodec_alloc_context3(eCodec);
       if (!eCodecCtx) {
           qWarning()<< "Couldn't init libfdk codectx.";
           return ;
       }
       eCodecCtx->profile=FF_PROFILE_AAC_LD;
       eCodecCtx->bit_rate = 128000;
       eCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
       eCodecCtx->sample_rate= 44100;
       eCodecCtx->channels=2;
       eCodecCtx->channel_layout=av_get_default_channel_layout(2);
       av_opt_set(eCodecCtx->priv_data, "tune", "zerolatency", 0);
       av_opt_set(eCodecCtx->priv_data, "preset", "ultrafast", 0);//快速
       //if (eCodecCtx->flags & AVFMT_GLOBALHEADER)
               eCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
       if ((ret=avcodec_open2(eCodecCtx, eCodec, NULL)) < 0) {
            qWarning()<<  "Could not open eCodecCtx" << ret;
           return ;
       }
       emit init_acc_stream(eCodec,eCodecCtx,0);
}

void CaptureAudio::encode_audio_frame(AVFrame *frame){
    if (frame) {
            frame->pts = pts;
            frame->time_base.num=1;
            frame->time_base.den=44100;
    }
    ret = avcodec_send_frame(eCodecCtx, frame);
    if (ret < 0) {
       qWarning()<<"Error sending the frame to the encoder"<<ret;
       return ;
    }
    qWarning()<<"encode audio pts is"<<frame->pts;
    while (ret >= 0) {
       AVPacket* pkt=av_packet_alloc();
       ret = avcodec_receive_packet(eCodecCtx, pkt);
       if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) continue;
       else if(ret<0) continue;
       qWarning()<<"encode pkt samplerate is";
       pkt->pts=frame->pts;
       pkt->dts=pkt->pts;
       pkt->time_base.num=1;
       pkt->time_base.den=44100;
       qWarning()<<pkt->size<<"cur send pkt size";
       emit send_pkt(pkt,0);
       qWarning()<<"发送";
    }
    pts += frame->nb_samples;
    qWarning()<<"encode pkt size down";
}

void CaptureAudio::OpenResample(){
    //actx分配空间
    actx = swr_alloc_set_opts(NULL,
        av_get_default_channel_layout(2),	//输出格式
        (AVSampleFormat)AV_SAMPLE_FMT_FLTP,			//输出样本格式 1 AV_SAMPLE_FMT_S16
        44100,					//输出采样率
        av_get_default_channel_layout(2),//输入格式
        (AVSampleFormat)AV_SAMPLE_FMT_S16,
        44100,
        0, 0
    );
    int re = swr_init(actx);
    if (re != 0)
    {
        qWarning()<<"swr init failed";
        return ;
    }
}

void CaptureAudio::run(){
    while(true && Stop_Quit==false)
    {
        AVPacket *pkt=(AVPacket *)av_malloc(sizeof(AVPacket));
        AVFrame	*pFrame=av_frame_alloc();
        if(av_read_frame(pFormatCtx,pkt)<0){
            av_packet_unref(pkt);
            qWarning()<<"抓取完毕";
            break;
        }
        avcodec_send_packet(pCodecCtx, pkt);
        av_frame_unref(pFrame);
        ret = avcodec_receive_frame(pCodecCtx, pFrame);

        if(ret==0){
            //libfdk
            //采集到的声音解码后 样本点大 需要 重采样(此处格式一致)后放入队列
            if(av_audio_fifo_size(fifo)<pFrame->nb_samples){
                ret=av_audio_fifo_realloc(fifo,av_audio_fifo_size(fifo)+pFrame->nb_samples);
            }
            //放入其中
            ret=av_audio_fifo_write(fifo,(void**)pFrame->data,pFrame->nb_samples);
            qWarning()<<"ret is"<<ret;
            while(av_audio_fifo_size(fifo)>0 && av_audio_fifo_size(fifo)>=AAC_FRMATE_SIZE){
               av_audio_fifo_read(fifo,(void**)eframe->data,AAC_FRMATE_SIZE);
               encode_audio_frame(eframe);
            }
        }else qWarning()<<"what fuck";
        //av_frame_free(&fltp_frame);
        av_packet_free(&pkt);
    }
}
