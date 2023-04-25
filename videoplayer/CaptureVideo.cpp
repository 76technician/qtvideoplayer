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
//    //The distance from the top edge of the screen or desktop
     av_dict_set(&options,"offset_y","40",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&options,"video_size","640x480",0);
    const AVInputFormat *ifmt=av_find_input_format("gdigrab");
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

void CaptureVideo::encode_init_context(){
    if(eCodec) return ;
    eCodec=avcodec_find_encoder_by_name("libx264");//找到h.264编码器
    if (!eCodec) {
        qWarning()<< "Couldn't init h.264 codec.";
        return ;
    }
    eCodecCtx=avcodec_alloc_context3(eCodec);
    if (!eCodecCtx) {
        qWarning()<< "Couldn't init h.264 codecontext.";
        return ;
    }
    eCodecCtx->bit_rate = 1000000; //一秒多少字节
    eCodecCtx->width = 640;
    eCodecCtx->height = 480;
    eCodecCtx->time_base.num=1;
    eCodecCtx->time_base.den=25;
    eCodecCtx->framerate.num=25;
    eCodecCtx->framerate.den=1;
    eCodecCtx->gop_size = 10; //根据此 会插入sps pps  捕获桌面不能为1 为1 则画质模糊
    eCodecCtx->max_b_frames = 0;
    eCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    //av_opt_set(eCodecCtx->priv_data, "preset", "slow", 0);
    av_opt_set(eCodecCtx->priv_data, "preset", "ultrafast", 0);//快速
    av_opt_set(eCodecCtx->priv_data, "tune", "zerolatency", 0);
    ret = avcodec_open2(eCodecCtx, eCodec, NULL);
    if (ret < 0) {
        qWarning()<< "Couldn't open h.264."<< ret;
        return ;
    }
    emit init_h264_stream(eCodec,eCodecCtx,1);
}

void CaptureVideo::encode_video_frame(AVFrame* frame){
    ret = avcodec_send_frame(eCodecCtx, frame);
    if (ret < 0) {
        qWarning()<<  "Error sending a frame for encoding"<<ret;
        return ;
    }
    while(ret>=0){
    AVPacket* pkt=av_packet_alloc();
    ret = avcodec_receive_packet(eCodecCtx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
        //qWarning()<<  "Error receive a pkt for encoding" << ret;
        av_packet_free(&pkt);
        return;
    }
    qDebug() << QStringLiteral("encode 编码后的pkt size is")<< pkt->size;
    pkt->pts=cur_pts;
    pkt->dts=pkt->pts;
    cur_pts++;
//    count++;
    qDebug() << pkt->pts << QStringLiteral("encode 编码后的 pkt");
//     fwrite(pkt->data,1,pkt->size,out);
//     fflush(out);
   emit send_pkt(pkt,1);
    }
    av_frame_free(&frame);
}

//使用avdevice dshow 捕获摄像头
void CaptureVideo::find_video_device(){
        avdevice_register_all();
        pFormatCtx = avformat_alloc_context();

        ifmt=av_find_input_format("dshow");
        AVDeviceInfoList* list;
        int devices_num=avdevice_list_input_sources(ifmt,"dshow",NULL,&list);
        QString cur_video;
        for(int i=0;i<devices_num;i++){
            if(*(list->devices[i]->media_types)==(AVMediaType::AVMEDIA_TYPE_VIDEO)){
            cur_video=list->devices[i]->device_description;
            emit add_device(cur_video);
            }
        }

        avdevice_free_list_devices(&list);
}

void CaptureVideo::video_codec(QString s){
        cur_pts=0;
        //打开设备
        char video_name[6+s.size()]="video=";
        int index=6;
        std::string cur=s.toStdString();
        for(auto c:cur) video_name[index++]=c;
        AVDictionary* options = NULL;
        av_dict_set(&options,"fps","25",0);
        ret=avformat_open_input(&pFormatCtx,video_name,ifmt,&options);
        if(ret!=0){
            qWarning()<< ret << "Couldn't open input stream.";
            return ;
        }
        ret=avformat_find_stream_info(pFormatCtx,NULL);
        if(ret<0)
        {
            qWarning()<<"Couldn't find stream information."<<ret;
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
        //emit video_play_init(pFormatCtx->streams[videoindex]->codecpar);//for loacl play

        //pCodecCtx->
        pCodec=avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);//视频解码器
        //for get avframe to convert encode
        pCodecCtx=avcodec_alloc_context3(pCodec);
        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar)) < 0) return ;
        pCodecCtx->thread_count=8;
        //打开解码器
        ret = avcodec_open2(pCodecCtx,pCodec,nullptr);
        if(ret < 0){
            qDebug() << QStringLiteral("打开解码器失败");
            return;
        }
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
        AVFrame	*pFrameYUYV=av_frame_alloc();
        if(av_read_frame(pFormatCtx,pkt)<0){
            av_packet_free(&pkt);
            av_frame_free(&pFrameYUYV);
            qWarning()<<"抓取完毕";
            continue;
        }
        ret = avcodec_send_packet(pCodecCtx, pkt);
        if (ret != 0){
             qDebug() << QStringLiteral("receive pkt from video is failed");
          }
         ret =avcodec_receive_frame(pCodecCtx, pFrameYUYV);
         if (ret != 0){
             qDebug() << QStringLiteral("receive pAVFrame is failed");
             av_frame_unref(pFrameYUYV);
             continue ;
        }
         pFrameYUYV->time_base.num=1;
         pFrameYUYV->time_base.den=25;
         AVFrame *pFrameYUV = av_frame_alloc();
         qDebug() << QStringLiteral("frame pts is ") << pFrameYUYV->pts;
        //av_packet_unref(pkt);
        //不是420p 转换
        struct SwsContext *img_convert_ctx = nullptr;
         qDebug() << pFrameYUYV->height <<pFrameYUYV->width << QStringLiteral("height width");

        img_convert_ctx = sws_getContext(
                pFrameYUYV->width, pFrameYUYV->height, (AVPixelFormat)pFrameYUYV->format, //输入
                640, 480, AV_PIX_FMT_YUV420P, //输出
                SWS_BICUBIC, nullptr, nullptr, nullptr);
        pFrameYUV->width=640;
        pFrameYUV->height=480;
        pFrameYUV->format=0;//0是YUV420p
        pFrameYUV->pts=pFrameYUYV->pts;
        pFrameYUV->time_base=pFrameYUYV->time_base;
        ret = av_frame_get_buffer(pFrameYUV, 0);
        sws_scale(img_convert_ctx, (uint8_t const **)pFrameYUYV->data,
                  pFrameYUYV->linesize, 0, pFrameYUYV->height, pFrameYUV->data,
                  pFrameYUV->linesize);
        av_frame_free(&pFrameYUYV);
        sws_freeContext(img_convert_ctx);
        if(pkt->stream_index==videoindex){
            // emit send_pkt(pkt);  //for loacl_play
            encode_video_frame(pFrameYUV);
            //发送给推流线程
            //emit send_pkt(pkt);
        }
        av_packet_free(&pkt);
    }
    qWarning()<<"capture is over";
}

