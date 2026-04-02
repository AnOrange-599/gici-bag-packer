#pragma once

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <unistd.h>
#include <cmath>
#include <time.h>
#include <eigen3/Eigen/Dense>
#include <glog/logging.h>
#include <ros/ros.h>

#include "constant.hpp"

// 引入GICI消息
#include <gici_datapacker/GnssObservations.h>
#include <gici_datapacker/GnssObservation.h>
#include <gici_datapacker/GnssEphemerides.h>
#include <gici_datapacker/GnssEphemeris.h>
#include <gici_datapacker/GlonassEphemeris.h>

using namespace std;

namespace gici_datapacker{
    /*
    输入参数sat：纯整数卫星编号；
    可选输出*prn，输入nullptr丢弃，或者输入 &变量，传入地址。
    返回值：卫星系统标志，比如SYS_GPS。（注意返回值是uint32_t，不是字符！）*/
    uint32_t satsys(uint32_t sat, uint32_t *prn);
    /*
    输入参数：uint32_t 卫星编号
    返回值：如G02字符串*/
    std::string sat2str(uint32_t sat_no);
    /*
    输入参数：G02字符串
    返回值：卫星编号uint32_t*/
    uint32_t str2sat(const std::string &sat_str);
    /*
    输入参数：卫星系统标记（SYS_GPS），系统内prn号
    返回值：卫星编号uint32_t */
    uint32_t sat_no(uint32_t sys, uint32_t prn);
    /*---------------------时间系统的转换---------------------------*/
    /*
    输入参数：原来的gtime_t类型时间，double类型的时间差
    返回值：相加后的gtime_t类型时间 */
    gtime_t time_add(gtime_t t, double sec);
    /*
    输入参数：减数t1，被减数t2，都是gtime_t类型
    返回值：t1-t2的double类型时间差 */
    double time_diff(gtime_t t1, gtime_t t2);
    /*
    输入参数：gtime_t类型的GPST时间
    返回值：gtime_t类型的UTC时间 */
    gtime_t gpst2utc(gtime_t t);

    /*
    输入参数：gtime_t类型的UTC时间
    返回值：gtime_t类型的GPST */
    gtime_t utc2gpst(gtime_t t);
    /*
    输入参数：GPST时间基准下的gtime_t类型时间
    返回值：BDT基准下的gtime_t类型时间 */
    gtime_t gpst2bdt (gtime_t t);
    /*
    输入参数：BDT基准下的gtime_t
    返回值：GPST基准下的gtime_t */
    gtime_t bdt2gpst (gtime_t t);
    /*
    输入参数：GPST基准下的gtime_t类型时间
    可选输出：北斗基准下的int *week整周数，注意调用需要传入地址；传入nullptr丢弃
    返回值：北斗基准下的周内秒double类型 */
    double gpst2bdt_week_tow(gtime_t t, int *week);
    /*
    输入参数：GPST基准下的gtime_t类型时间
    可选输出：GPST基准下的int *week整周数，注意调用需要传入地址；传入nullptr丢弃
    返回值：GPST基准下的double类型周内秒 */
    double gpst2gpst_week_tow(gtime_t t, int *week);
    /*
    输入参数：GPST基准下的gtime_t类型时间
    可选输出：Galileo系统时间（GST）基准下的int *week整周数，注意调用需要传入地址；传入nullptr丢弃
    返回值：Galileo系统时间（GST）基准下的double类型周内秒（TOW）
    GST与GPST秒长一致且连续对齐 */
    double gpst2galileo_week_tow(gtime_t t, int *week);
    /*
    输入参数：GPST时间基准下的周+周内秒
    输出参数：函数声明中使用引用传递！直接修改！传入的周+周内秒为BDT基准下的周+周内秒！
    调用直接传入值即可
    无返回值 */
    void gpst2bdt (int &week, double &tow);
    /*
    输入参数：BDT时间基准下的周+周内秒
    输出参数：函数声明中使用引用传递！直接修改！传入的周+周内秒为GPST基准下的周+周内秒！
    调用直接传入值即可
    无返回值 */
    void bdt2gpst (int &week, double &tow);
    /* 
    输入参数：double *ep，传入 &ep[0] 即可，也就是第1个元素的地址
    返回值：GPST基准下的gtime_t类型 */
    gtime_t epoch_array2gpst(const double *ep);
    /*
    输入参数：gtime_t类型的GPST时间
    输出参数：double *ep  day/time {year,month,day,hour,min,sec} */
    void gpst2epoch_array(gtime_t t, double *ep);
    /*
    GPST转Unix时间，因为ROS消息使用的是Unix时间基准
    输入参数：GPST基准下的gtime_t
    返回值：Unix基准下的gtime_t */
    gtime_t gpst2unix (gtime_t t);


    /*----------------------变量->GICI消息的转换--------------------------*/
    /*
    输入参数：观测值列表
    返回值：一条完整的GnssObservations消息 */
    GnssObservations obs2msg(const vector<ObsPtr>& obs_list);
    /*
    输入参数：一个G/C/E的星历
    返回值：单条星历消息GnssEphemeris */
    GnssEphemeris ephem2msg(EphemPtr ephem);
    /*
    输入参数：一个R的星历
    返回值：单条星历消息GlonassEphemeris */
    GlonassEphemeris glo_ephem2msg(GloEphemPtr ephem);

    /*
    找到已经发布的最新星历
    输入参数：一条观测值，观测值的卫星对应的星历列表
    输出参数：通过引用修改resultEphem，也就是找到的最新（且已发布）星历*/
    void FindNewEphem(ObsPtr& obs,vector<EphemBasePtr>& ephems,EphemBasePtr& resultEphem);
    


}