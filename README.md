# qtvideoplayer

a video player or video and auido real-time call  by qt-quick and ffmpeg(for encode,decode ,rtsp packet send or receive and capture device)
development environment： QT 6.2.4  ffmpeg-n5.1-shared
build by QtCreadtor MinGW_64_bit qmake

lib--ffmpeg's shared lib

include--ffmpeg's h files

RunDll-- necessary running files

### videoplayer --this player's body

   CaptureVideo(thread) capture video or screen and send packed encoded by h264 to rtsppush

   CaptureAudio(thread) capture audio from micphone or your sound card  and send packed encoded by acc to rtsppush

   Demuxed(thread) demux media file and send avpacket to videoThread ,send pcm to AudioItem(In fact,the consistency of video and audio's true by audio's pts)**（only loca file）**
   
   VideoThread(thread) decode video frame,convert video frame to QImage,final displayed on the VideoItem
   
   VideoItem It receive QImage and drawer it in QVideoFrame's buff,and via VideoSink to diaplay every VideoFrame
   
   AudioThread(thread) decode audio frame,convert audio frame to pcm raw data **(only online's file)**
   
   AudioItem It receive pdm,and send data to AudioDevice's buff.



Demuxed's job is support Audio and Video synchronization in local file,but rtsppull and rtsppush'job is support Audio and Video synchronization in online file.Besides,you can't  send packet to your receive rtsp address at same time.You also can't transport your rtsp packet by udp.Maybe this issue be created by ffmpeg. But even so,you still can create a video or audio appication,it's delay just 1second.

If you want to develop such program or debug,you need add RunDll's files  to your running's root directory.


