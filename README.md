# qtvideoplayer
a video player by qt-quick and ffmpeg(for encode,decode and capture device)
development environment： QT 6.2.3  ffmpeg-n4.4-shared

lib--ffmpeg's shared lib

include--ffmpeg's h files

RunDll-- necessary running files

videoplayer
           --this player's body

           --CaptureVideo(thread) capture video or screen and send avpacket to videoThread
           
           --Demuxed(thread) demux media file and send avpacket to videoThread ,send pcm to AudioItem(In fact,the consistency of video and audio's true by audio's pts)
           
           --VideoThread(thread) decode video frame,convert video frame to QImage,final displayed on the VideoItem
           
           --VideoItem It receive QImage and drawer it in QVideoFrame's buff,and via VideoSink to diaplay every VideoFrame
           
           --AudioItem It receive pdm,and send data to AudioDevice's buff.
           

If you want to develop such program or debug,you need add RunDll's files  to your running's root directory.

