import QtQuick
import Qt.labs.platform
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtMultimedia
import Producer
import Demuxed
import AudioPlay
import VideoThread
import CaptureVideo
ApplicationWindow   {
    id:top_window
    width: 1000
    height: 650
    visible: true
    title: qsTr("my player")
        //此处窗口移动就会停止窗口内所有事件 需要自定义标题栏
     flags: Qt.Window | Qt.FramelessWindowHint   //去标题栏
     //记录鼠标移动的位置，此处变量过多会导致移动界面变卡
         property point  clickPos: "0,0"
         //自定义标题栏
     Rectangle{
         id: mainTitle
         width: top_window.width
         anchors.left: parent.left
         height: 15
         color: '#1677b3'

         //处理鼠标移动后窗口坐标逻辑
         MouseArea{
             anchors.fill: parent
             acceptedButtons: Qt.LeftButton  //只处理鼠标左键
             onPressed: function(mouse){    //鼠标左键按下事件
                 clickPos = Qt.point(mouse.x, mouse.y)
             }
             onPositionChanged: function (mouse){    //鼠标位置改变
                 //计算鼠标移动的差值
                 var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                 //设置窗口坐标
                 top_window.setX(top_window.x + delta.x)
                 top_window.setY(top_window.y + delta.y)
             }
         }

         //关闭窗口按钮
         Image {
             id: closeButton
             anchors.right: parent.right
             width: 30
             height: 15
              Rectangle{
                  width: 30
                  height: 15
                  color: '#131124'
              }

             MouseArea{
                 anchors.fill: parent
                 onClicked: {
                        Qt.quit()        //退出程序
                 }
             }
         }
 }
    //自定义qml类型
    Demuxed{
        id:demux
    }
    Producer{
        id: producer
        videoSink: videoOutput.videoSink
    }
    AudioPlay{
        id: aduioplay
    }
    VideoThread{
        id:video_thread
    }
    CaptureVideo{
        id: capturevideo
    }

    FileDialog {
         id: fileDialog
         title: "打开图片或者txt文件"
         nameFilters: ["Video files(*.mp4 *.avi)", "All files (*)"]
         acceptLabel: "确定"
         rejectLabel: "取消"
         fileMode: FileDialog.OpenFile
         onAccepted: {
             console.log("选中的文件有:")
             //获得文件路径名字 传递给ffmpeg 打开
             //解封装 将帧传给 producer
             console.log(file)
//             fileselect.visible=false
             demux.format_init(file)
//             videoview.width=top_window.width
//             videoview.height=top_window.height
             demux.demuxed()
             demux.video_decode()
             demux.audio_decode()
             videoslider.visible=true
             video_thread.open()
             demux.open()
             video_thread.start()
             video_thread.set_mod(0)
             demux.start()
         }
     }
    //button for file Dialog
    Rectangle{
         id: fileselect
         anchors.right:parent.right
         anchors.rightMargin: 25
         anchors.bottom: parent.bottom
         anchors.bottomMargin: 25
         color: '#475164'
         Text{
             text: "选择"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: fileDialog.open();
         }
     }
    //button for quit play thread
    Rectangle{
         id: threadquit
         anchors.right:fileselect.left
         anchors.rightMargin: 25
         anchors.bottom: parent.bottom
         anchors.bottomMargin: 25
         color: '#475164'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  videoslider.visible=false;
                  demux.stop()
                  video_thread.stop()
                  video_thread.terminate()
                  capturevideo.terminate()
              }
         }
     }
    //button for capture device
    Rectangle{
         id: capture
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: fileselect.top
         anchors.bottomMargin: 25
         color: '#1661ab'
         Text{
             text: "发送"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.find_video_device()
                  video_thread.open()
                  video_thread.set_mod(1)
                  capturevideo.open()
                  video_thread.start()
                  capturevideo.start()
              }
         }
     }
    //button for quit capture Video
    Rectangle{
         id: capturequit
         anchors.right:capture.left
         anchors.rightMargin: 25
         anchors.bottom: threadquit.top
         anchors.bottomMargin: 25
         color: '#1661ab'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.stop()
                  capturevideo.terminate()
                  video_thread.stop()
                  video_thread.terminate()
              }
         }
     }
    //button for capture develop
    Rectangle{
         id: record
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: capture.top
         anchors.bottomMargin: 25
         color: '#f0d695'
         Text{
             text: "录制"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.record_screen_init()
                  capturevideo.open()
                  video_thread.open()
                  video_thread.set_mod(1)
                  video_thread.start()
                  capturevideo.start()
              }
         }
     }
    //button for quit develop
    //button for quit capture Video
    Rectangle{
         id: developequit
         anchors.right:record.left
         anchors.rightMargin: 25
         anchors.bottom: capturequit.top
         anchors.bottomMargin: 25
         color: '#f0d695'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 43
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.stop()
                  capturevideo.terminate()
                  video_thread.stop()
                  video_thread.terminate()
              }
         }
     }
    Connections{
             target: demux
             function onSend_frame(frame){
                  //更改sink中帧
                 //producer.handleTimeout(data)
                 producer.paint_frame(frame)
                 console.log("收到frame emit")
             }
             function onFormat_solt(sampleRate,sampleSize,channel){
                 console.log("audio format emit")
                 aduioplay.setFormat(sampleRate,sampleSize,channel)
             }
             function onSend_pcm(data,len,cur_len){
                 console.log("audio data receive")
                 aduioplay.play_sound(data,len);
                 videoslider.value=cur_len
             }
             function onVideo_init(para,time_base,sum){
                 video_thread.video_encode_init(para,time_base)
                 videoslider.visible=true
                 videoslider.to=sum;
             }
             function onSync(mesc){
                 console.log("audio set video sync")
                 video_thread.setSync(mesc);
             }
             function onSend_pkt(pkt){
                 console.log("video data receive")
                 video_thread.receive_pkt(pkt);
             }
     }
    Connections{
         target: video_thread
         function onSend_image(image){
             producer.paint_frame(image)
             console.log("收到vido frame emit")
         }
     }
    Connections{
         target:  capturevideo
         function onVideo_play_init(pCodeP){
            video_thread.video_encode_init(pCodeP,0)
         }
         function onSend_pkt(pkt){
            video_thread.receive_pkt(pkt);
         }
     }

    Rectangle {
         //视频输出框
         id: videoview
         x:0
         y:31
         width: 640
         height: 480
         visible: true
         VideoOutput{
             id: videoOutput
             anchors.fill: parent
         }
     }
    Slider{
        id: videoslider
        visible: false
        anchors.top: videoview.bottom
        anchors.left: parent.left
        from:0
        width:videoview.width
        live:false
        //滑块
        handle: Rectangle{
          id:handle_area
          x: videoslider.leftPadding + videoslider.visualPosition * (videoslider.availableWidth - width)
          y: videoslider.topPadding + videoslider.availableHeight / 2 - height / 2
          width:20
          height:20
          color:'#1ba784'
          radius: 10
        }
        //背景
        background: Rectangle {
            x: videoslider.leftPadding
            y: videoslider.topPadding + videoslider.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 4
            width:videoview.width
            height: implicitHeight
            radius: 2
            color: '#7a7374'
        }
        onPressedChanged: {
            console.log("i am free"+value);
            //发送信号给demux线程 videothread线程
            video_thread.seek(value)
            demux.change_process(value)
        }
        }
}
