#pragma once

#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <rinex_helper.hpp>
#include <constant.hpp>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
//处理图片需要OpenCV库
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>




namespace gici_datapacker{

    struct IMUIterator {
        ifstream file; //记录读取位置的文件指针
        double t, ax, ay, az, wx, wy, wz;//7个参数
        bool valid = false;//默认为false，标记为数据无效，防止未初始化就被使用。
        int seq = 0;
    };
    struct ImageIterator {
        std::ifstream csv_file;      // 读 header.csv（跳过标题行）的指针
        cv::VideoCapture video_cap;  // 读 video.mp4
        double t;                   //当前帧对应的时间
        std::string frame_id;
        uint32_t width, height;
        std::string encoding;
        cv::Mat current_frame; //当前视频帧
        bool valid = false;
    };


    //IMU相关函数
    void init_imu_iterator(IMUIterator& it, const std::string& path);
    void read_next_imu(IMUIterator& it);
    sensor_msgs::Imu imu2msg(IMUIterator& it);
    //图像相关函数
    void init_image_iterator(ImageIterator& it, const std::string& csv_path, 
        const std::string& video_path);
    void read_next_image(ImageIterator& it);
    sensor_msgs::Image image2msg(const ImageIterator& it);


}