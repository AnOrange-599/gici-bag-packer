cmake_minimum_required(VERSION 2.8.3)
project(gici_datapacker)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_BUILD_TYPE "debug")
set(CMAKE_CXX_FLAGS "-std=c++14 -O3 -Wall -fPIC")

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  message_generation
  rosbag
  cv_bridge
)

# 下面这些是GVINS提供的msg文件
add_message_files(
  DIRECTORY gvins_msg
  FILES
    GnssTimeMsg.msg
    GnssEphemMsg.msg
    GnssGloEphemMsg.msg
    GnssMeasMsg.msg
    GnssObsMsg.msg
    GnssBestXYZMsg.msg
    GnssPVTSolnMsg.msg
    GnssSvsMsg.msg
    GnssTimePulseInfoMsg.msg
    StampedFloat64Array.msg
)
# 下面这些是GICI提供的msg文件
add_message_files(
  DIRECTORY gici_msg
  FILES
    GlonassEphemeris.msg
    GnssObservation.msg
    GnssSsrEphemeris.msg
    GnssAntennaPosition.msg
    GnssObservations.msg
    GnssSsrPhaseBias.msg
    GnssEphemerides.msg
    GnssSsrCodeBias.msg
    GnssSsrPhaseBiases.msg
    GnssEphemeris.msg
    GnssSsrCodeBiases.msg
    GnssIonosphereParameter.msg
    GnssSsrEphemerides.msg
)
# 生成消息
generate_messages(DEPENDENCIES std_msgs)

# 声明ROS包依赖（仅保留必要的消息运行时依赖）
catkin_package(CATKIN_DEPENDS message_runtime)

# 引入第三方库（Eigen3+Glog）
find_package(Eigen3 REQUIRED)
# find_package(Glog REQUIRED)
find_package(OpenCV REQUIRED)

# 头文件包含目录
include_directories(
  ${EIGEN3_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${catkin_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include/
  /usr/local/include
)

# 构建可执行文件
set(SRC_FILES
    src/main_packer.cpp
    src/utility.cpp
    src/rinex_helper.cpp
    src/VI.cpp
)

add_executable(raw2bag ${SRC_FILES})



# 链接依赖库（核心：${catkin_LIBRARIES}已包含rosbag+roscpp等所有ROS库）
target_link_libraries(raw2bag
  ${catkin_LIBRARIES}
  /usr/local/lib/libglog.so
  ${OpenCV_LIBS} 
  # Threads::Threads
)


# 确保消息生成完成后再编译可执行文件
add_dependencies(raw2bag ${PROJECT_NAME}_generate_messages_cpp)
