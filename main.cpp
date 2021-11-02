#include <iostream>
#include <opencv2/opencv.hpp>
#include "wiringSerial.h"
#include <unistd.h>

using namespace std;
using namespace cv;

#define TRI_THRESH 1500

uint8_t SAVING_IMAGE = 0;  //采集图片
uint8_t THRESHOLD = 0;     //测阈值
//
uint8_t blue = 1, red = 1;
uint8_t color = 0;
//轮廓面积
int AREA_THRESHOLD = 800;
int iLowH = 0;
int iHighH = 5;

int iLowS = 89;
int iHighS = 254;

int iLowV = 136;
int iHighV = 255;

Scalar RED_LOWER(0,92,48); //HSV   0  89   136
Scalar RED_UPPER(5,255,255);
Scalar BLUE_LOWER(100,115,134);
Scalar BLUE_UPPER(125,255,255);

int main(){

    VideoCapture cap(0);
    if(!cap.isOpened()){
        cout<<"open camera error!"<<endl;
        return -1;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH,400);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,300);
    cap.set(cv::CAP_PROP_AUTO_EXPOSURE,1.00);//修改成手动曝光0.25
    cap.set(CV_CAP_PROP_EXPOSURE,0.02);

    int i = 0;

    Mat frame,red_dst,blue_dst,imgHSV,dst;
    char send_data[4] = {0};
    vector<vector<Point>> contours;

    //vector<Vec4i> hierarchy;
    int fd;
    if ((fd = serialOpen("/dev/ttyAMA1", 115200)) < 0)
    {
        //fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
        cout<<"open serial error"<<endl;
        return 1;
    }

    if(THRESHOLD){
        namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

        //Create trackbars in "Control" window
        cvCreateTrackbar("LowH", "Control", &iLowH, 179);
        cvCreateTrackbar("HighH", "Control", &iHighH, 179);

        cvCreateTrackbar("LowS", "Control", &iLowS, 255);
        cvCreateTrackbar("HighS", "Control", &iHighS, 255);

        cvCreateTrackbar("LowV", "Control", &iLowV, 255);
        cvCreateTrackbar("HighV", "Control", &iHighV, 255);
    }

    while(1){
        memset(send_data, 0, sizeof(send_data));
        cap.read(frame);
       // cout<<frame.cols<<frame.rows<<endl;//352 288
        Mat ROI1 = frame(Rect(0,0,100,288));
        Mat ROI2 = frame(Rect(252,0,100,288));
        //Mat ROI3 = frame(Rect(200,0,400,150));
        //Mat ROI4 = frame(Rect(200,450,400,150));

        ROI1 =Scalar(0,0,0);
        ROI2 =Scalar(0,0,0);
        //ROI3 =Scalar(0,0,0);
        //ROI4 =Scalar(0,0,0);

        //imshow("b",frame);
        //waitKey(10);
        //continue;
        blue = red = 1;
        color = 0;
        if(SAVING_IMAGE){
            char key = waitKey(10);
            if(key == 'k' || key == 'K'){
                cout<<i<<endl;
                imwrite("../img/" + to_string(i) + ".jpg",frame);
                i++;
            }
            else if(key == 'x'){
                return -1;
            }
            imshow("原像",frame);
            waitKey(10);
            continue;
        }

        //cout<<"图像处理"<<endl;
        if(THRESHOLD){
            cvtColor(frame,imgHSV,CV_BGR2HSV);
            inRange(imgHSV,Scalar(iLowH,iLowS,iLowV),Scalar(iHighH,iHighS,iHighV),dst);
            Mat element = getStructuringElement(MORPH_RECT,Size(30,30));
            //erode(red_dst,red_dst,element);

            morphologyEx(dst,dst,MORPH_CLOSE,element);
            Mat ele = getStructuringElement(MORPH_RECT,Size(3,3));
            dilate(dst,dst,ele);
            imshow("threshold",dst);
            imshow("video",frame);
            waitKey(10);
            continue;
        }

        if(red){
            contours.clear();
            cvtColor(frame,imgHSV,CV_BGR2HSV);
            inRange(imgHSV,RED_LOWER,RED_UPPER,red_dst);
            Mat element = getStructuringElement(MORPH_RECT,Size(30,30));
            morphologyEx(red_dst,red_dst,MORPH_CLOSE,element);
            //morphologyEx(red_dst,red_dst,MORPH_OPEN,element);

            Mat ele = getStructuringElement(MORPH_RECT,Size(1,1));
            dilate(red_dst,red_dst,ele);
            dilate(red_dst,red_dst,ele);
            //erode(red_dst,red_dst,ele);
            //morphologyEx(red_dst,red_dst,MORPH_OPEN,ele);

            findContours(red_dst,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
            //imshow("red",red_dst);
            //imshow("src",frame);
            //waitKey(10);
            //cout<<"continue"<<endl;
            //continue;

            for(int i = 0;i < contours.size();i++){

                int area = contourArea(contours[i]);
                cout<<area<<endl;
                if (area < AREA_THRESHOLD) continue;
                color = 1;
                //imshow("red",red_dst);
                //waitKey(10);
                //cout<<"red"<<endl;
                blue = 0;

            }

            if(blue){
                contours.clear();
                cvtColor(frame,imgHSV,CV_BGR2HSV);
                inRange(imgHSV,BLUE_LOWER,BLUE_UPPER,blue_dst);
                Mat element = getStructuringElement(MORPH_RECT,Size(20,20));
                morphologyEx(blue_dst,blue_dst,MORPH_CLOSE,element);
                //morphologyEx(red_dst,red_dst,MORPH_OPEN,element);

                Mat ele = getStructuringElement(MORPH_RECT,Size(1,1));
                dilate(blue_dst,blue_dst,ele);
                dilate(blue_dst,blue_dst,ele);
                findContours(blue_dst,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
                //imshow("blue",blue_dst);
                //waitKey(10);
                for(int i=0;i<contours.size();i++){
                    int area = contourArea(contours[i]);
                    cout<<area<<endl;
                    if(area<AREA_THRESHOLD) continue;
                    color = 2;
                    //imshow("blue",blue_dst);
                    //waitKey(10);
                   // cout<<"blue"<<endl;
                }
            }
        }

        //cout<<"color:"<<color<<endl;
        if(color == 2 || color == 1){
            vector<vector<Point>> contours_ploy(contours.size());
            //cout<< contours.size()<<endl;
            for(int i = 0 ;i < contours.size(); i++){
                int area = contourArea(contours[i]);
                if(area > AREA_THRESHOLD){
                   // float peri = arcLength(contours[i],true);
                    float peri = 300;
                    //cout<<peri<<endl;
                    approxPolyDP(contours[i],contours_ploy[i],0.02*peri, true);
                    drawContours(frame,contours_ploy,i,Scalar(0,255,0),2);
                    //cout<< contours_ploy[i].size()<<endl;
                   // imshow("object",frame);
                    //waitKey(10);
                    int object = (int)contours_ploy[i].size();
                    if(object == 3 && color == 1 && area < TRI_THRESH){

                        cout<<"RedTriangle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x05;//5
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);
                    }

                    else if(object == 3 && color == 2 && area < TRI_THRESH){
                        cout<<"BlueTriangle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x06;//6
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);

                    }
                    else if(object == 4 && color == 1) {
                        cout<<"RedRectangle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x03;//3
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);
                    }

                    else if(object == 4 && color == 2){
                        cout<<"BlueRectangle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x04;//4
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);
                    }
                    else if(object > 4 && color ==1){
                        cout<<"RedCircle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x01;//1
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);
                    }
                    else if(object > 4 && color ==2){
                        cout<<"BlueCircle"<<endl;
                        *(signed char *) &send_data[0] = 0x2C;
                        *(signed char *) &send_data[1] = 0x12;
                        *(signed char *) &send_data[2] = 0x02;//2
                        *(signed char *) &send_data[3] = 0x5B;
                        write(fd,send_data,4);
                    }
                }
            }
        }
        else{
            cout<<"None"<<endl;
            *(signed char *) &send_data[0] = 0x2C;
            *(signed char *) &send_data[1] = 0x12;
            *(signed char *) &send_data[2] = 0x00;
            *(signed char *) &send_data[3] = 0x5B;
            write(fd,send_data,4);
        }
        waitKey(10);//skip;
        //cout<<"application runs ......"<<endl;
    }

    return 0;

}