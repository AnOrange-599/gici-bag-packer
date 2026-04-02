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
    /*输入参数：观测文件的文件路径
    返回值：卫星系统对应的观测值类型字典，比如'G'对应：C1C L1C D1C S1C C2S L2S D2S S2S
    注意卫星系统键是GCER单字符 */
    map<char, vector<string>> readRinexObsHeader(const string& path);
    /*解析时间戳行
    返回值是时间戳，和这一历元的观测值个数*/
    gtime_t rinexEphocLine2Ephoc(const string ephoc_str, int &num_obs);
    /*解析单行观测值*/
    ObsPtr rinexObsLine2Obs(const string rinex_str, const map<char, vector<string>> &sys2type);
    /*读取整个导航文件*/
    void rinex2ephems(const std::string &rinex_filepath, std::map<uint32_t, std::vector<EphemBasePtr>> &sat2ephem);
    /*解析单个GLONASS星历数据块*/
    GloEphemPtr rinex_line2glo_ephem(std::vector<std::string> ephem_lines);
    /*解析单个GCE星历数据块*/
    EphemPtr rinex_line2ephem(std::vector<std::string> ephem_lines);
}