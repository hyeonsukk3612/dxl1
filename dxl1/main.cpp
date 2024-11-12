#include "opencv2/opencv.hpp"
#include <iostream>
using namespace cv;
using namespace std;

int main()
{
    string src = "nvarguscamerasrc sensor-id=0 ! \
        video/x-raw(memory:NVMM), width=(int)640, height=(int)360, \
    format=(string)NV12, framerate=(fraction)30/1 ! \
        nvvidconv flip-method=0 ! video/x-raw, \
        width=(int)640, height=(int)360, format=(string)BGRx ! \
        videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    // NVIDIA Jetson 카메라에서 영상을 캡처

    VideoCapture source(src, CAP_GSTREAMER);
    if (!source.isOpened()){ cout << "Camera error" << endl; return -1; }
    // 카메라 소스 열기 및 오류 확인

    string dst1 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8001 sync=false";
    // 원본 영상

    VideoWriter writer1(dst1,0, (double)30,Size(640,360),true);
    if(!writer1.isOpened()) {cerr<<"Writer open failed!"<<endl; return -1;}
    // 원본 영상 스트리밍

    string dst2 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8002 sync=false";
    // 그레이스케일 영상

    VideoWriter writer2(dst2,0, (double)30,Size(640,360),false);
    if(!writer2.isOpened()) {cerr<<"Writer open failed!"<<endl; return -1;}
    // 그레이스케일 영상 스트리밍

    string dst3 = "appsrc ! videoconvert ! video/x-raw, format=BGRx ! \
        nvvidconv ! nvv4l2h264enc insert-sps-pps=true ! \
        h264parse ! rtph264pay pt=96 ! \
        udpsink host=203.234.58.168 port=8003 sync=false";
    // 이진화 영상

    VideoWriter writer3(dst3,0, (double)30,Size(640,360),false);
    if(!writer3.isOpened()) {cerr<<"Writer open failed!"<<endl; return -1;}
    // 이진화 영상 스트리밍

    Mat frame, gray, thread;
    TickMeter timer;
    
    while (true) {
        timer.start();
        source >> frame;  // 카메라에서 프레임 읽기
        if (frame.empty()){ cerr << "frame empty!" << endl; break; }
        cvtColor(frame, gray, COLOR_BGR2GRAY);  // 그레이스케일 변환
        threshold(gray, thread, 128, 255, THRESH_BINARY);  // 이진화 처리
        writer1 << frame;  // 원본 영상 스트리밍
        writer2 << gray;   // 그레이스케일 영상 스트리밍
        writer3 << thread; // 이진화 영상 스트리밍
        waitKey(30);  // 30ms 대기
        timer.stop();
		cout<<"time : "<<timer.getTimeMilli()<<"ms"<<endl;
		timer.reset();
    }
    return 0;
}

