# GICI Data Packer

[] 2026年6月4日，为了后续参加比赛，暂时删掉了一部分源码。之后会恢复完整代码。

## 项目简介

GICI Data Packer 是一个用于将 GNSS 观测数据、IMU 数据和图像数据打包成**GICI可运行的 ROS bag 文件**的工具。支持**处理 RINEX 格式的 GNSS 观测文件和导航文件**，以及 IMU 数据和视频数据，将它们转换为 ROS 消息并保存到 bag 文件中，以便于后续的 GNSS/INS 融合处理。

本项目为个人学习实践代码，基于特定环境与示例数据编写，未经全面测试，可能存在疏漏，仅供学习参考，敬请谅解。

## 功能特性

- 支持处理 RINEX 3.04版本的 GNSS 观测文件（.obs）和导航文件（.nav）
- 支持 GPS、GLONASS、Galileo 和 BeiDou 卫星系统
- 支持时间系统转换（GPST、BDT、UTC、Unix）
- IMU和视频的时间戳文件格式是**我自己（随便🤐）定义的**，建议自行修改VI.cpp中的代码，或者按照我提供的文本格式进行组织。

### IMU和视频的时间戳文件格式
- IMU 数据文件（.csv）：每行包含以下字段：
  ```
  timestamp,ax,ay,az,wx,wy,wz
  ```
  - timestamp: 时间戳（秒）
  - ax, ay, az: 加速度计数据（m/s²）
  - wx, wy, wz: 陀螺仪数据（rad/s）
- 视频数据（.mp4）：每帧包含以下字段：
  ```
  timestamp,frame_number,width,height,encoding
  ```
  - timestamp: 时间戳（秒）
  - frame_number: 视频帧序号
  - width, height: 视频帧的宽度和高度
  - encoding: 视频帧的编码格式，黑白图像为"mono8"，彩色图像为"bgr8"



### 自定义文本格式需要修改的代码

只需要修改VI.cpp中的`read_next_image`和`read_next_imu`函数即可
这两个是**解析单行文本的函数**，根据自己的文本格式进行修改即可。

## 依赖项

- Ubuntu 20.04 + ROS Kinetic
- Eigen 3.4：[https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz](https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz)
- Glog 0.6.0：[https://codeload.github.com/google/glog/tar.gz/refs/tags/v0.6.0](https://codeload.github.com/google/glog/tar.gz/refs/tags/v0.6.0)
- OpenCV 4.2.0（有一个依赖库包含了OpenCV4.2.0，所以不需要单独下载、编译、安装）
- C++14 或更高版本

## 安装步骤

<workspace> 替换成自己的路径

1. 创建 ROS 工作空间（如果尚未创建）：
   ```bash
   mkdir -p <workspace>/GICI_DataPacker_ws/src
   cd <workspace>/GICI_DataPacker_ws
   catkin_make
   ```

2. 克隆本仓库到 ROS 工作空间的 src 目录：
   ```bash
   cd <workspace>/GICI_DataPacker_ws/src
   git clone https://gitee.com/Orange_867/gici-bag-packer.git
   ```

3. 编译项目：
   ```bash
   cd <workspace>/GICI_DataPacker_ws
   catkin_make
   ```

## 使用方法

1. 准备数据文件，按照以下结构组织（**注意文件名有严格要求！**）：
   ```
   data/
   ├── rinex.obs     # GNSS 观测文件
   ├── rinex.nav     # GNSS 导航文件
   ├── imu.csv       # IMU 数据文件
   └── Image/
       ├── header.csv  # 图像时间戳文件
       └── video.mp4   # 视频文件
   ```

2. 修改 `src/main_packer.cpp` 中的路径定义（如果需要）：
   ```cpp
   // 默认数据输入路径，可根据实际情况修改
   #define INPUT_FILEPATH "./data"
   // 默认输出路径，可根据实际情况修改
   #define OUTPUT_FILEPATH "./output"
   ```

3. 运行程序：
   ```bash
   cd <workspace>/GICI_DataPacker_ws
   source devel/setup.bash
   rosrun gici_datapacker raw2bag
   ```

4. 程序会在 `output/` 目录下生成以下 bag 文件（注意**output目录要提前建好**）：
   ```
   output/
   ├── obs.bag     # GNSS 观测数据
   ├── nav.bag     # GNSS 星历数据
   ├── imu.bag     # IMU 数据
   └── image.bag   # 图像数据
   ```

## 示例数据

包含视频、IMU、GNSS观测值、星历的原始数据，和使用本程序生成的bag文件。

[示例数据](https://pan.baidu.com/s/1kL4P3-WxEseHYRrkpWV2bQ?pwd=fmuv)


## 注意事项

- 程序默认使用相对路径 `./data` 作为数据输入目录，`./output` 作为输出目录
- 请确保输入数据格式正确，否则可能导致程序运行失败
- 程序处理时间系统转换，使用的是**混合格式**的星历和观测文件，可能需要根据实际情况，修改时间系统转换的代码

## 参考

部分代码借鉴了香港科技大学曹绍祖的 gnss_comm 和 RTKLIB

## 许可证

本项目采用 GPLv3 许可证
