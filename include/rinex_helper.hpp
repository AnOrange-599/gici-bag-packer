#pragma once


#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include <time.h>
#include <glog/logging.h>

#include "constant.hpp"
#include "utility.hpp"


using namespace std;

namespace gici_datapacker{
    /*
    读取RINEX观测文件头
    输入参数：
    - path: 观测文件的文件路径
    返回值：
    - 卫星系统对应的观测值类型字典，比如'G'对应：C1C L1C D1C S1C C2S L2S D2S S2S
    注意：卫星系统键是GCER单字符
    */
    std::map<char, std::vector<std::string>> readRinexObsHeader(const std::string& path);
    
    /*
    解析RINEX观测文件中的时间戳行
    输入参数：
    - ephoc_str: 时间戳行字符串
    - num_obs: 输出参数，这一历元的观测值个数
    返回值：
    - gtime_t类型的时间戳
    */
    gtime_t rinexEphocLine2Ephoc(const std::string ephoc_str, int &num_obs);
    
    /*
    解析单行RINEX观测值
    输入参数：
    - rinex_str: 观测值行字符串
    - sys2type: 卫星系统对应的观测值类型字典
    返回值：
    - ObsPtr类型的观测数据智能指针
    */
    ObsPtr rinexObsLine2Obs(const std::string rinex_str, const std::map<char, std::vector<std::string>> &sys2type);
    
    /*
    读取整个RINEX导航文件
    输入参数：
    - rinex_filepath: 导航文件路径
    - sat2ephem: 输出参数，卫星编号到星历列表的映射
    */
    void rinex2ephems(const std::string &rinex_filepath, std::map<uint32_t, std::vector<EphemBasePtr>> &sat2ephem);
    
    /*
    解析单个GLONASS星历数据块
    输入参数：
    - ephem_lines: 星历数据块的行向量
    返回值：
    - GloEphemPtr类型的GLONASS星历智能指针
    */
    GloEphemPtr rinex_line2glo_ephem(std::vector<std::string> ephem_lines);
    
    /*
    解析单个GPS/Galileo/BeiDou星历数据块
    输入参数：
    - ephem_lines: 星历数据块的行向量
    返回值：
    - EphemPtr类型的星历智能指针
    */
    EphemPtr rinex_line2ephem(std::vector<std::string> ephem_lines);
}