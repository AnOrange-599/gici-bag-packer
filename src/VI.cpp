#include "VI.hpp"
using namespace std;

namespace gici_datapacker{

    void init_imu_iterator(IMUIterator& it, const string& path) {
        it.file.open(path);
        // it.valid = false;//确保一开始数据状态无效，防止未初始化就使用。
        string header_line;
        getline(it.file, header_line);    // 跳过 CSV 标题行
        read_next_imu(it);  // 读第一条
    }
    void read_next_imu(IMUIterator& it) {
        string line;
        if (getline(it.file, line) && !line.empty()) {
            replace(line.begin(), line.end(), ',', ' ');
            istringstream ss(line);
            ss >> it.t >> it.ax >> it.ay >> it.az >> it.wx >> it.wy >> it.wz;
            // it.wx*=M_PI/180.0; it.wy*=M_PI/180.0; it.wz*=M_PI/180.0;
            it.valid = true;
        }
        else
            it.valid = false;
    }
    sensor_msgs::Imu imu2msg(IMUIterator& it){
        sensor_msgs::Imu msg;
        msg.header.stamp = ros::Time(it.t);
        msg.header.frame_id = "imu0";
        msg.header.seq = it.seq++;
        msg.linear_acceleration.x = it.ax;
        msg.linear_acceleration.y = it.ay;
        msg.linear_acceleration.z = it.az;
        msg.angular_velocity.x = it.wx;
        msg.angular_velocity.y = it.wy;
        msg.angular_velocity.z = it.wz;

        return msg;
    }

    void init_image_iterator(ImageIterator& it, const string& csv_path, 
        const string& video_path) {
        it.csv_file.open(csv_path);
        it.video_cap.open(video_path);
        // it.valid = false;

        // 跳过 CSV 标题行
        string header_line;
        getline(it.csv_file, header_line);

        read_next_image(it); // 读第一帧 + 第一行数据
    }
    void read_next_image(ImageIterator& it) {
        string line;
        if (getline(it.csv_file, line) && it.video_cap.read(it.current_frame)) {
            // 转换为灰度单通道图像
            cv::cvtColor(it.current_frame, it.current_frame, cv::COLOR_BGR2GRAY);
            // 解析 CSV 行
            replace(line.begin(), line.end(), ',', ' ');
            istringstream ss(line);
            ss >> it.t >> it.frame_id >> it.width >> it.height >> it.encoding;
            it.valid = true;
        } else {
            it.valid = false;
        }
    }
    sensor_msgs::Image image2msg(const ImageIterator& it){
        sensor_msgs::Image msg;
        msg.header.stamp = ros::Time(it.t);  // 时间戳
        msg.header.frame_id = "cam0";
        msg.height = it.height;              // 图像高度
        msg.width = it.width;                // 图像宽度
        msg.encoding = it.encoding;          // 编码格式，这里是灰度图像mono8
        msg.step = it.width;             // 每行字节数，如果是彩色，需要设置为it.width*3

        size_t size = msg.height * msg.step;
        msg.data.resize(size);               // 调整 data 大小

        if (!it.current_frame.empty()) {
            memcpy(msg.data.data(), it.current_frame.data, size);
        }

        return msg; 
    }
}