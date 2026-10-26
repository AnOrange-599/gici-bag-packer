#include "constant.hpp"
#include "utility.hpp"
#include "rinex_helper.hpp"
#include <ros/ros.h>
#include <rosbag/bag.h>
#include <rosbag/view.h>
#include "VI.hpp"
using namespace std;
using namespace gici_datapacker;

// 默认数据输入路径，可根据实际情况修改，data是文件夹名
#define INPUT_FILEPATH "/root/gici/GICI_DataPacker_ws/src/gici_datapacker/data"
// 默认输出路径，可根据实际情况修改
#define OUTPUT_FILEPATH "/root/gici/GICI_DataPacker_ws/src/gici_datapacker/output"
// 每多少条观测，添加一次星历消息
#define EPHEM_INTERVAL 50

map<char, vector<string>> sys2type;
vector<vector<ObsPtr>> obs_epochs_list; // 所有历元的观测值列表，注意是二维列表
map<uint32_t, vector<EphemBasePtr>> sat2ephem;
namespace gici_datapacker{bool use_native_time = false;} // 头文件中声明，cpp文件中定义



int main(int argc, char** argv){
    google::InitGoogleLogging(argv[0]);//初始化glog
    // 拼接字符串
    string obs_filepath = string(INPUT_FILEPATH)+"/rinex.obs";
    string nav_filepath = string(INPUT_FILEPATH)+"/rinex.nav";
    string ref_filepath = string(INPUT_FILEPATH)+"/reference.obs";
    string imu_path = string(INPUT_FILEPATH)+"/imu.csv";
    string video_header_path = string(INPUT_FILEPATH)+"/Image/header.csv";
    string video_path = string(INPUT_FILEPATH)+"/Image/video.mp4";
    
    // GnssAntennaPosition position_msg; //TODO: 如何赋值到时候再说
    double position[3] = {-2747160.5604, 4764418.0561, 3219357.9739}; //TODO: 参考站的ECEF坐标，有时间记得改过来
    
    //处理GNSS观测值
    sys2type = readRinexObsHeader(obs_filepath); // 读取文件头，得到观测值类型字典
    ifstream rinex_file; rinex_file.open(obs_filepath); // 从头开始读取文件，把文件头跳过
    string rinex_line;
    // 跳过文件头
    while (getline(rinex_file, rinex_line))
        if(rinex_line.find("END OF HEADER") != string::npos)
            break;
    while(getline(rinex_file, rinex_line)){
        if(rinex_line.at(0) != '>') // 不是时间戳行，则跳过
            continue;
        int num_obs=0;
        gtime_t ephoc = rinexEphocLine2Ephoc(rinex_line, num_obs);
        vector<ObsPtr> obs_single_ephoc_list; // 单个历元的观测值列表
        for(int i=0;i<num_obs;i++){//遍历这一历元每一行观测值
            getline(rinex_file, rinex_line);
            ObsPtr obs = rinexObsLine2Obs(rinex_line, sys2type);
            obs->time = ephoc;//记得给观测值的时间戳赋值
            int week;
            obs->tow = gpst2gpst_week_tow(ephoc, &week);
            obs->week = week;
            obs_single_ephoc_list.emplace_back(obs);
        }
        obs_epochs_list.emplace_back(obs_single_ephoc_list); // 保存这单个历元的数据
    }
    //将观测值列表打包进入bag
    rosbag::Bag obs_bag; obs_bag.open(string(OUTPUT_FILEPATH)+"/obs.bag", rosbag::bagmode::Write); // 打开bag文件
    for(vector<ObsPtr> ephoc:obs_epochs_list){
        GnssObservations msg = obs2msg(ephoc);
        obs_bag.write("/gici/gnss_rover/observations", msg.header.stamp, msg);
    }

    //处理参考站数据
    ifstream reference_file; reference_file.open(ref_filepath); // 读取参考站观测文件的指针
    vector<vector<ObsPtr>> ref_epochs_list; // 所有历元的观测值列表，注意是二维列表
    //检查文件是否存在
    if(reference_file.is_open()){
        map<char, vector<string>> ref_sys2type = readRinexObsHeader(ref_filepath); // 读取文件头，得到观测值类型字典
        string ref_line;
        while(getline(reference_file, ref_line)){
            if(ref_line.at(0) != '>') // 不是时间戳行，则跳过
                continue;
            int num_obs=0;
            gtime_t ephoc = rinexEphocLine2Ephoc(ref_line, num_obs); // 读取时间戳行，得到观测值数量
            if(time_diff(ephoc, obs_epochs_list[0][0]->time) < 0.0) continue; // 如果参考站观测早于流动站观测，则跳过
            if(time_diff(ephoc, obs_epochs_list.back().back()->time) > 0.0) break; // 如果参考站观测晚于流动站观测，直接结束
            vector<ObsPtr> ref_single_ephoc_list; // 单个历元的观测值列表
            for(int i=0;i<num_obs;i++){//遍历这一历元每一行观测值
                getline(reference_file, ref_line);
                ObsPtr obs = rinexObsLine2Obs(ref_line, ref_sys2type);
                obs->time = ephoc;//记得给观测值的时间戳赋值
                int week;
                obs->tow = gpst2gpst_week_tow(ephoc, &week);
                obs->week = week;
                ref_single_ephoc_list.emplace_back(obs); // 保存这单个历元的数据

            }
            ref_epochs_list.emplace_back(ref_single_ephoc_list); // 保存这单个历元的数据
        }
        rosbag::Bag ref_bag; ref_bag.open(string(OUTPUT_FILEPATH)+"/reference.bag", rosbag::bagmode::Write); // 打开bag文件
        int ref_index = 0;
        for(vector<ObsPtr> ephoc:ref_epochs_list){
            GnssObservations msg = obs2msg(ephoc);
            ref_bag.write("/gici/gnss_reference/observations", msg.header.stamp, msg);
            if(ref_index++ % 10 == 0){
                GnssAntennaPosition position_msg;
                position_msg.header.stamp = msg.header.stamp;
                position_msg.header.frame_id = "gnss";
                position_msg.pos.resize(3);
                position_msg.pos[0] = position[0];
                position_msg.pos[1] = position[1];
                position_msg.pos[2] = position[2];
                ref_bag.write("/gici/gnss_reference/antenna_position", msg.header.stamp, position_msg);
            }
        }
        ref_bag.close();
        (void)system(("rosbag info " + string(OUTPUT_FILEPATH) + "/reference.bag").c_str());

    }
    

    // 接下来是打包星历的尝试
    rinex2ephems(nav_filepath, sat2ephem);//读取整个文件并存入字典
    rosbag::Bag nav_bag; nav_bag.open(string(OUTPUT_FILEPATH)+"/nav.bag", rosbag::bagmode::Write); // 打开bag文件
    int obs_index = 0;
    for(vector<ObsPtr> ephoc:obs_epochs_list){
        if(obs_index++ % EPHEM_INTERVAL != 0 && obs_index > 30) continue; // 每50次观测添加一次星历，前30次观测不做限制
        GnssEphemerides ephemrides_msg;
        // 给ROS消息时间戳赋值
        gtime_t unix_t = gpst2unix(ephoc[0]->time);
        ephemrides_msg.header.stamp = ros::Time(
            unix_t.time,  // 整数秒
            static_cast<uint32_t>(std::round(unix_t.sec * 1e9))  // 小数秒转纳秒
        );
        ephemrides_msg.header.frame_id = "gnss"; // frame_id
        for (auto &kv : sat2ephem) { // kv 是 key-value，也就是遍历键值对的意思
            int sat = kv.first;

            // 检查卫星是否是GPS、GLONASS、GALILEO、BDS，不是则直接跳过
            uint32_t sys = satsys(sat, nullptr);
            if(sys != SYS_GPS && sys != SYS_GLO && sys != SYS_GAL && sys != SYS_BDS) continue;

            vector<EphemBasePtr> &sat_ephem_list = kv.second;

            // 找到该时刻最新星历
            EphemBasePtr result_ephem = make_shared<EphemBase>();
            FindNewEphem(ephoc[0], sat_ephem_list, result_ephem);  // 注意传入时间

            // 检查是否找到有效星历（例如 prn 非空）
            if (result_ephem->prn.empty()) continue;

            char sys_char = result_ephem->prn[0];
            if (sys_char == 'R') {
                auto ephem = dynamic_pointer_cast<GloEphem>(result_ephem);
                if (ephem) 
                    ephemrides_msg.glonass_ephemerides.push_back(glo_ephem2msg(ephem));
            } 
            else if (sys_char == 'C' || sys_char == 'G' || sys_char == 'E') {
                auto ephem = dynamic_pointer_cast<Ephem>(result_ephem);
                if (ephem) 
                    ephemrides_msg.ephemerides.push_back(ephem2msg(ephem));
            }
        }
        nav_bag.write("/gici/gnss_ephemeris/ephemerides", ephemrides_msg.header.stamp, ephemrides_msg);
    }


    //最后处理一下VI打包
    IMUIterator imu_it;
    init_imu_iterator(imu_it,imu_path);
    rosbag::Bag imu_bag; imu_bag.open(string(OUTPUT_FILEPATH)+"/imu.bag", rosbag::bagmode::Write);
    while (imu_it.valid){
        imu_bag.write("/gici/imu_raw", ros::Time(imu_it.t), imu2msg(imu_it));
        // printf("写入了一个IMU消息，时间戳为%.4f\n",ros::Time(imu_it.t).toSec());
        read_next_imu(imu_it);
    }
    
    ImageIterator img1_it;
    init_image_iterator(img1_it,video_header_path,video_path);
    rosbag::Bag image_bag; image_bag.open(string(OUTPUT_FILEPATH)+"/image.bag", rosbag::bagmode::Write);
    while(img1_it.valid){
        image_bag.write("/gici/image_raw", ros::Time(img1_it.t), image2msg(img1_it));
        read_next_image(img1_it);   
    }




    imu_bag.close();
    image_bag.close();
    obs_bag.close();
    nav_bag.close();
    // ref_bag.close(); // 因为有可能没有参考数据

    // 输出bag文件信息
    (void)system(("rosbag info " + string(OUTPUT_FILEPATH) + "/obs.bag").c_str());
    (void)system(("rosbag info " + string(OUTPUT_FILEPATH) + "/nav.bag").c_str());
    (void)system(("rosbag info " + string(OUTPUT_FILEPATH) + "/imu.bag").c_str());
    (void)system(("rosbag info " + string(OUTPUT_FILEPATH) + "/image.bag").c_str());
}



