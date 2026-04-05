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

    /*
    IMU数据迭代器结构体
    用于读取和管理IMU数据文件，并通过其内容写入Bag文件
    */
    struct IMUIterator {
        ifstream file; //记录读取位置的文件指针
        double t, ax, ay, az, wx, wy, wz;//7个参数，角度单位都是rad
        bool valid = false;//默认为false，标记为数据无效，防止未初始化就被使用。
        int seq = 0;
    };
    
    /*
    图像数据迭代器结构体
    用于读取和管理图像数据文件，分别包含了读取时间戳文本文件和视频帧的指针
    */
    struct ImageIterator {
        std::ifstream csv_file;      // 读取header.csv文件的指针（跳过标题行）
        cv::VideoCapture video_cap;  // 读取video.mp4文件的视频捕获对象
        double t;                   // 当前帧对应的时间戳（秒）
        std::string frame_id;       // 帧标识符
        uint32_t width;             // 图像宽度
        uint32_t height;            // 图像高度
        std::string encoding;       // 图像编码格式
        cv::Mat current_frame;      // 当前视频帧
        bool valid;                 // 数据有效性标志，默认为false
    };


    /*
    初始化IMU迭代器
    输入参数：
    - it: IMU迭代器对象引用
    - path: IMU数据文件路径
    */
    void init_imu_iterator(IMUIterator& it, const std::string& path);
    
    /*
    读取下一个IMU数据
    输入参数：
    - it: IMU迭代器对象引用，读取后会更新其中的数据
    */
    void read_next_imu(IMUIterator& it);
    
    /*
    将IMU数据转换为ROS消息
    输入参数：
    - it: IMU迭代器对象引用，包含要转换的数据
    返回值：
    - sensor_msgs::Imu类型的ROS消息
    */
    sensor_msgs::Imu imu2msg(IMUIterator& it);
    
    /*
    初始化图像迭代器
    输入参数：
    - it: 图像迭代器对象引用
    - csv_path: header.csv文件路径
    - video_path: video.mp4文件路径
    */
    void init_image_iterator(ImageIterator& it, const std::string& csv_path, 
        const std::string& video_path);
    
    /*
    读取下一个图像数据
    输入参数：
    - it: 图像迭代器对象引用，读取后会更新其中的数据
    */
    void read_next_image(ImageIterator& it);
    
    /*
    将图像数据转换为ROS消息
    输入参数：
    - it: 图像迭代器对象引用，包含要转换的数据
    返回值：
    - sensor_msgs::Image类型的ROS消息
    */
    sensor_msgs::Image image2msg(const ImageIterator& it);


}