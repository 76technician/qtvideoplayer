#include <CaptureVideo.h>

//使用avdevice gdigrab 抓取桌面
void CaptureVideo::record_screen_init(){
    avdevice_register_all();
    //Use gdigrab
    AVDictionary* options = NULL;
    pFormatCtx = avformat_alloc_context();
    //Set some options
    //grabbing frame rate
     av_dict_set(&options,"framerate","25",0);
    //The distance from the left edge of the screen or desktop
     av_dict_set(&options,"offset_x","20",0);
    //The distance from the top edge of the screen or desktop
     av_dict_set(&options,"offset_y","40",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&options,"video_size","640x480",0);
    AVInputFormat *ifmt=av_find_input_format("gdigrab");
    if(avformat_open_input(&pFormatCtx,"desktop",ifmt,&options)!=0){
        qWarning() << "Couldn't open input stream(screen record).\n";
        return ;
    }
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        qWarning()<<"Couldn't find stream information.";
        return ;
    }
    video_codec();
}

//使用avdevice dshow 捕获摄像头
void CaptureVideo::find_video_device(){
        avdevice_register_all();
        pFormatCtx = avformat_alloc_context();
        ifmt=av_find_input_format("dshow");
        if(avformat_open_input(&pFormatCtx,"video=USB2.0 VGA UVC WebCam",ifmt,NULL)!=0){
            qWarning()<< "Couldn't open input stream.";
            return ;
        }
        if(avformat_find_stream_info(pFormatCtx,NULL)<0)
        {
            qWarning()<<"Couldn't find stream information.";
            return ;
        }
        video_codec();
}

void CaptureVideo::video_codec(){
        videoindex=-1;
        uint i;
        for(i=0; i<pFormatCtx->nb_streams; i++)
            if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                videoindex=i;
                break;
            }
        if(videoindex==-1)
        {
            qWarning()<<"Couldn't find a video stream.";
            return ;
        }
        pCodec=avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);
        if(pCodec==NULL)
        {
            qWarning()<<"Codec not found.";
            return ;
        }
        pCodecCtx=avcodec_alloc_context3(pCodec);
        if ( avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar) < 0) {
            qWarning()<<"can't convert parameters to context.";
            return ;
         }
        if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
        {
            qWarning()<<"Could not open pCodecCtx.";
            return ;
        }
        emit video_play_init(pFormatCtx->streams[videoindex]->codecpar);
}
void CaptureVideo::open(){
    Stop_Quit=false;
}
void CaptureVideo::stop(){
    Stop_Quit=true;
    avformat_close_input(&pFormatCtx);
    avcodec_close(pCodecCtx);
}
void CaptureVideo::run(){
    while(true && Stop_Quit==false)
    {
        AVPacket *pkt=(AVPacket *)av_malloc(sizeof(AVPacket));
        if(av_read_frame(pFormatCtx,pkt)<0){
            av_packet_unref(pkt);
            qWarning()<<"抓取完毕";
            break;
        }
        if(pkt->stream_index==videoindex){
             emit send_pkt(pkt);
        }else av_packet_unref(pkt);
    }
    qWarning()<<"capture is over";
}

