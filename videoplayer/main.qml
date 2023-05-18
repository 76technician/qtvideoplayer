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
import CaptureAudio
import RtspPush
import RtspPull
import AudioThread

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
    AudioThread{
        id:audio_thread
    }
    CaptureVideo{
        id: capturevideo
    }
    CaptureAudio{
        id:captureaudio
    }
    RtspPush{
        id:rtsppush
    }
    RtspPull{
        id:rtsppull
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
    //select camera device
    ComboBox {
        id: select_box
        anchors.right:parent.right
        anchors.rightMargin: 25
        anchors.top: parent.top
        anchors.topMargin: 25
        editable:true
        model: ListModel{
            id: model
           ListElement { text: " " }
        }
        delegate: ItemDelegate { //呈现标准视图项 以在各种控件和控件中用作委托
          width: select_box.width
          contentItem: Text {
              text: modelData   //即model中的数据
              color: "red"
              font: select_box.font
              verticalAlignment: Text.AlignVCenter
          }
        }
        contentItem: Text { //界面上显示出来的文字
          leftPadding: 5 //左部填充为5
          text: select_box.displayText //表示ComboBox上显示的文本
          font: select_box.font    //文字大小
          color: select_box.pressed ? "orange" : "black"   //文字颜色
          verticalAlignment: Text.AlignVCenter  //文字位置
        }
        background: Rectangle {   //背景项
          implicitWidth: 120
          implicitHeight: 40
          color: "#70f3ff"
          border.width: 1
          radius: 2
        }
        onActivated:  {
            console.log("已选择",currentText)
            //select_box.visible=false
            capturevideo.open()
            capturevideo.video_codec(currentText)
            capturevideo.start()
//            rtsppush.open()
//            rtsppush.start()
//            video_thread.open()
//            video_thread.set_mod(1)
//            video_thread.start()
        }
    }

    ComboBox {
        id: select_box_two
        anchors.right:parent.right
        anchors.rightMargin: 25
        anchors.top: select_box.bottom
        anchors.topMargin: 25
        editable:true
        model: ListModel{
            id: device_text
           ListElement { text: " " }
        }
        delegate: ItemDelegate { //呈现标准视图项 以在各种控件和控件中用作委托
          width: select_box_two.width
          contentItem: Text {
              text: modelData   //即model中的数据
              color: "red"
              font: select_box_two.font
              verticalAlignment: Text.AlignVCenter
          }
        }
        contentItem: Text { //界面上显示出来的文字
          leftPadding: 5 //左部填充为5
          text: select_box_two.displayText //表示ComboBox上显示的文本
          font: select_box_two.font    //文字大小
          color: select_box_two.pressed ? "orange" : "black"   //文字颜色
          verticalAlignment: Text.AlignVCenter  //文字位置
        }
        background: Rectangle {   //背景项
          implicitWidth: 120
          implicitHeight: 40
          color: "#70f3ff"
          border.width: 1
          radius: 2
        }
        onActivated:  {
            console.log("已选择",currentText)
            //select_box.visible=false
            captureaudio.audio_codec(currentText)
            rtsppush.open()
            rtsppush.start()
            captureaudio.open()
            captureaudio.start()
        }
    }

    TextField{
        id:rtsp_address
        anchors.left: parent.left
        anchors.leftMargin: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        placeholderText:qsTr("rtsp://xxx.xxx.xxx.xxx")
        font.family: "Microsoft YaHei"
        font.pixelSize: 14
        width:250
        height:32
        background: Rectangle {
            border.color: "black"
        }
    }
    Rectangle{
        id: enter_add
        anchors.left:rtsp_address.right
        anchors.leftMargin: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        color: '#2b73af'
        Text{
            text: "pull确定"
            anchors.centerIn: parent
        }
        width: 50
        height: 25
        MouseArea{
             anchors.fill: parent
             onClicked: {
                 rtsppull.set_rtsp_address(rtsp_address.text)
                 rtsppush.set_rtsp_address(rtsp_address.text)
                 console.log("sent rtsp address");
            }
        }
    }
    Rectangle{
        id: enter_adds
        anchors.left:enter_add.right
        anchors.leftMargin: 25
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 15
        color: '#2b73af'
        Text{
            text: "push确定"
            anchors.centerIn: parent
        }
        width: 50
        height: 25
        MouseArea{
             anchors.fill: parent
             onClicked: {
                 rtsppush.set_rtsp_address(rtsp_address.text)
                 console.log("sent rtsp address");
            }
        }
    }

    //button for file Dialog
    Rectangle{
         id: fileselect
         anchors.right:parent.right
         anchors.rightMargin: 25
         anchors.bottom: parent.bottom
         anchors.bottomMargin: 25
         color: '#2b73af'
         Text{
             text: "选择"
             anchors.centerIn: parent
         }
         width: 45
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
         color: '#eea2a4'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 45
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
    //button for capture  cam device
    Rectangle{
         id: capture
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: fileselect.top
         anchors.bottomMargin: 25
         color: '#2b73af'
         Text{
             text: "发送视频"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.find_video_device()
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
         color: '#eea2a4'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.stop()
                  capturevideo.terminate()
                  rtsppush.stop()
                  rtsppush.terminate()
              }
         }
     }
    //button for capture  audio device
    Rectangle{
         id: audio
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: capture.top
         anchors.bottomMargin: 25
         color: '#2b73af'
         Text{
             text: "发送音频"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  captureaudio.find_audio_device()
                  captureaudio.encode_init_context()
                  capturevideo.encode_init_context()
              }
         }
     }
    //button for quit capture audio
    Rectangle{
         id: audioquit
         anchors.right:capture.left
         anchors.rightMargin: 25
         anchors.bottom: capture.top
         anchors.bottomMargin: 25
         color: '#eea2a4'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  captureaudio.stop()
                  captureaudio.terminate()
                  rtsppush.stop()
                  rtsppush.terminate()
              }
         }
     }
    //button for receive rtsp  stream
    Rectangle{
         id: rtsp_pull
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: audio.top
         anchors.bottomMargin: 25
         color: '#2b73af'
         Text{
             text: "拉流"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  video_thread.open()
                  video_thread.set_mod(0)
                  video_thread.start()
                  audio_thread.open()
                  audio_thread.start()
                rtsppull.rtsp_init()
                rtsppull.open()
                rtsppull.start()
              }
         }
     }
    //button for quit recevie rtsp stream
    Rectangle{
         id: rtsp_pull_quit
         anchors.right:capture.left
         anchors.rightMargin: 25
         anchors.bottom: audio.top
         anchors.bottomMargin: 25
         color: '#eea2a4'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                rtsppull.stop()
                rtsppull.terminate()
                video_thread.stop()
                video_thread.terminate()
                audio_thread.stop()
                audio_thread.terminate()
              }
         }
     }


    //button for capture develop
    Rectangle{
         id: record
         anchors.right: parent.right
         anchors.rightMargin: 25
         anchors.bottom: rtsp_pull.top
         anchors.bottomMargin: 25
         color: '#2b73af'
         Text{
             text: "发送桌面"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.record_screen_init()
                  capturevideo.open()
                  capturevideo.start()
              }
         }
     }
    //button for quit capture Video
    Rectangle{
         id: developequit
         anchors.right:record.left
         anchors.rightMargin: 25
         anchors.bottom: rtsp_pull.top
         anchors.bottomMargin: 25
         color: '#eea2a4'
         Text{
             text: "终止"
             anchors.centerIn: parent
         }
         width: 45
         height: 25
         MouseArea{
              anchors.fill: parent
              onClicked: {
                  capturevideo.stop()
                  capturevideo.terminate()
                  rtsppush.stop()
                  rtsppush.terminate()
              }
         }
     }
    Connections{
             target: demux
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
             target: rtsppull
//             function onFormat_solt(sampleRate,sampleSize,channel){
//                 console.log("audio format emit")
//                 aduioplay.setFormat(sampleRate,sampleSize,channel)
//             }
//             function onSend_pcm(data,len,cur_len){
//                 console.log("audio data receive")
//                 aduioplay.play_sound(data,len);
//                 videoslider.value=cur_len
//             }
             function onAudio_rtsp_init(para){
                audio_thread.audio_decode(para)
             }
             function onVideo_rtsp_init(para,time_base,sum){
                 video_thread.video_encode_init(para,time_base)
                 videoslider.visible=true
                 videoslider.to=sum;
             }
//             function onRtsp_sync(mesc){
//                 console.log("audio set video sync")
//                 video_thread.setSync(mesc);
//             }
             function onRtsp_send_pkt(pkt,f){
                if(f===0){
                 console.log("video data receive")
                 video_thread.receive_pkt(pkt);
                }
                if(f===1){
                console.log("audio data receive")
                audio_thread.receive_pkt(pkt);
                }
             }
     }
    Connections{
        target:audio_thread
        function onFormat_solt(sampleRate,sampleSize,channel){
            console.log("audio format emit")
            aduioplay.setFormat(sampleRate,sampleSize,channel)
        }
        function onSend_pcm(data,len,cur_len){
            console.log("audio data receive")
            aduioplay.play_sound(data,len);
            videoslider.value=cur_len
        }
        function onRtsp_sync(mesc){
         console.log("audio set video sync")
         video_thread.setSync(mesc);
        }
        function onSet_nb_samples(a){
            rtsppull.set_nb_samples(a);
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
//         function onSend_pkt(pkt){
//            video_thread.receive_pkt(pkt);
//         }
         function onAdd_device(s){
            console.log("设置 device 成功")
            select_box.model.append({text: s})
         }
         function onSend_pkt(pkt,f){
            console.log("传入 视频帧")
            rtsppush.receive_pkt(pkt,f)
         }
         function onInit_h264_stream(eCodec,eCodecCtx,f){
              console.log("配置输出流 video")
             rtsppush.init_out_stream(eCodec,eCodecCtx,f)
         }
     }
    Connections{
        target: captureaudio
        function onFormat_solt(sampleRate,sampleSize,channel){
            console.log("audio format emit")
            aduioplay.setFormat(sampleRate,sampleSize,channel)
        }
        function onSend_pcm(data,len){
            console.log("audio data receive")
            aduioplay.play_sound(data,len);
        }
        function onAdd_device(s){
           console.log("设置 device 成功")
           select_box_two.model.append({text: s})
        }
        function onSend_pkt(pkt,f){
           console.log("传入 音频帧")
           rtsppush.receive_pkt(pkt,f)
        }
        function onInit_acc_stream(eCodec,eCodecCtx,f){
           console.log("配置输出流 audio")
           rtsppush.init_out_stream(eCodec,eCodecCtx,f)
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
