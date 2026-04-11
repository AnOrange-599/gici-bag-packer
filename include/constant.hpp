#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <iomanip>
#include <eigen3/Eigen/Dense>

namespace gici_datapacker{

    // 定义频率值常量
    #define FREQ1       1.57542E9           /* L1/E1  frequency (Hz) */
    #define FREQ2       1.22760E9           /* L2     frequency (Hz) */
    #define FREQ5       1.17645E9           /* L5/E5a frequency (Hz) */
    #define FREQ6       1.27875E9           /* E6/LEX frequency (Hz) */
    #define FREQ7       1.20714E9           /* E5b    frequency (Hz) */
    #define FREQ8       1.191795E9          /* E5a+b  frequency (Hz) */
    #define FREQ9       2.492028E9          /* S      frequency (Hz) */
    #define FREQ1_GLO   1.60200E9           /* GLONASS G1 base frequency (Hz) */
    #define DFRQ1_GLO   0.56250E6           /* GLONASS G1 bias frequency (Hz/n) */
    #define FREQ2_GLO   1.24600E9           /* GLONASS G2 base frequency (Hz) */
    #define DFRQ2_GLO   0.43750E6           /* GLONASS G2 bias frequency (Hz/n) */
    #define FREQ3_GLO   1.202025E9          /* GLONASS G3 frequency (Hz) */
    #define FREQ1_BDS   1.561098E9          /* BeiDou B1 frequency (Hz) */
    #define FREQ2_BDS   1.20714E9           /* BeiDou B2 frequency (Hz) */
    #define FREQ3_BDS   1.26852E9           /* BeiDou B3 frequency (Hz) */

    //各个卫星系统的数量，用于prn和卫星编号转换
    //GPS
    #define MIN_PRN_GPS  1
    #define MAX_PRN_GPS 32
    #define N_SAT_GPS   (MAX_PRN_GPS - MIN_PRN_GPS + 1)
    //GLONASS
    #define MIN_PRN_GLO  1
    #define MAX_PRN_GLO 27
    #define N_SAT_GLO   (MAX_PRN_GLO - MIN_PRN_GLO + 1)
    //GALILEO
    #define MIN_PRN_GAL  1
    #define MAX_PRN_GAL 38
    #define N_SAT_GAL   (MAX_PRN_GAL - MIN_PRN_GAL + 1)
    //BDS
    #define MIN_PRN_BDS  1
    #define MAX_PRN_BDS 63
    #define N_SAT_BDS   (MAX_PRN_BDS - MIN_PRN_BDS + 1)
    //卫星总数
    #define MAX_SAT     (N_SAT_GPS + N_SAT_GLO + N_SAT_GAL + N_SAT_BDS)

    //辨别每一个卫星系统的编号，比如：if(sys==SYS_GPS)
    #define SYS_NONE    0x00                /* navigation system: none */
    #define SYS_GPS     0x01                /* navigation system: GPS */
    #define SYS_SBS     0x02                /* navigation system: SBAS */
    #define SYS_GLO     0x04                /* navigation system: GLONASS */
    #define SYS_GAL     0x08                /* navigation system: Galileo */
    #define SYS_QZS     0x10                /* navigation system: QZSS */
    #define SYS_BDS     0x20                /* navigation system: BeiDou */
    #define SYS_IRN     0x40                /* navigation system: IRNSS */
    #define SYS_LEO     0x80                /* navigation system: LEO */
    #define SYS_ALL     0xFF                /* navigation system: all */

    // 字符对应系统
    const std::map<uint8_t, uint32_t> char2sys = 
    {
        {'G', SYS_GPS},
        {'C', SYS_BDS},
        {'R', SYS_GLO},
        {'E', SYS_GAL}
    };
    const std::map<uint8_t, uint32_t> sys2char = 
    {
        {SYS_GPS, 'G'},
        {SYS_BDS, 'C'},
        {SYS_GLO, 'R'},
        {SYS_GAL, 'E'}
    };

    //2017年之后，闰秒数一直是18
    #define LEAP_SECONDS 18

    //各系统的时间起点
    const static double gpst0[] = {1980,1,6,0,0,0}; /* gps time reference */
    const static double gst0 [] = {1999,8,22,0,0,0}; /* galileo system time reference */
    const static double bdt0 [] = {2006,1,1,0,0,0}; /* beidou time reference */

    //标记是否使用了原生时间系统
    extern bool use_native_time;// 默认使用了GPST

    //各卫星系统的频率列表，及其对应的具体数值
    const std::map<std::string, double> type2freq = 
    {
        {"G1", FREQ1},
        {"G2", FREQ2},
        {"G5", FREQ5},
        {"R1", FREQ1_GLO},
        {"R2", FREQ2_GLO},
        {"R3", 1.202025E9},
        {"R4", 1.600995E9},
        {"R6", 1.248060E9},
        {"E1", FREQ1},
        {"E5", FREQ5},
        {"E6", FREQ6},
        {"E7", FREQ7},
        {"E8", FREQ8},
        {"C1", FREQ1},
        {"C2", FREQ1_BDS},
        {"C5", FREQ5},
        {"C6", FREQ3_BDS},
        {"C7", FREQ2_BDS},
        {"C8", FREQ8}
    };

    // 根据 GICI 映射表定义支持的频率（不含系统字母）
    // GICI支持的GPS频率列表
    const std::set<std::string> SUPPORTED_SUFFIX_G = {"1C","1S","1L","1X","1P","1W","1Y","1M","2C","2D","2S","2L","2X","2P","2W","2Y","2M","5I","5Q","5X"};
    // GICI支持的GLONASS频率列表
    const std::set<std::string> SUPPORTED_SUFFIX_R = {"1C","1P","4A","4B","4X","2C","2P","6A","6B","6X","3I","3Q","3X"};
    // GICI支持的Galileo频率列表
    const std::set<std::string> SUPPORTED_SUFFIX_E = {"1A","1B","1C","1X","1Z","5I","5Q","5X","7I","7Q","7X","8I","8Q","8X","6A","6B","6C","6X","6Z"};
    // GICI支持的BeiDou频率列表
    const std::set<std::string> SUPPORTED_SUFFIX_C = {"2I","2Q","2X","1D","1P","1X","1S","1L","1Z","5D","5P","5X","7I","7Q","7X","7D","7P","7Z","8D","8P","8X","6I","6Q","6X","6Z"};





    /*
    时间结构体
    用于表示精确时间，包含整秒部分和小数部分
    */
    struct gtime_t
    {
        time_t time;            /* 整秒部分，使用标准time_t类型表示，其实就是一个long类型的别名 */
        double sec;             /* 秒的小数部分，小于1秒 */
    };

    /*
    星历基类
    所有卫星系统（GCER）星历的基类，包含通用的星历信息
    */
    struct EphemBase
    {
        virtual ~EphemBase() = default;
        std::string prn;
        uint16_t week;
        uint8_t iode; // 数据龄期
        uint8_t svh; // 健康状态


        uint32_t sat; // 卫星编号
        gtime_t header_t; // 我也不知道第1行到底是什么时间，就随便起一个吧

    };
    typedef std::shared_ptr<EphemBase> EphemBasePtr;  /* 星历基类智能指针 */

    /*
    GLONASS星历结构体
    继承自EphemBase，包含GLONASS特有的星历信息
    */
    struct GloEphem : EphemBase
    {
        uint8_t         frq;            // 卫星频率编号 (GLONASS频点号，范围1-24)
        uint8_t         age;            // 星历龄期 (单位：小时，表征星历数据的新鲜度)
        double          pos[3];         // 卫星位置
        double          vel[3];         // 卫星速度
        double          acc[3];         // 卫星加速度
        double          taun, gamn;     // taun：卫星钟偏(单位：秒)；gamn：卫星相对频率偏置
        double          dtaun;          // L1和L2载波之间的硬件延迟
        double          toe,tof;        // toe：星历参考时刻(Time of Ephemeris)；tof：星历消息接收时刻(Time of Frame)。注意！GLONASS的参考时刻是日内秒，
    };
    typedef std::shared_ptr<GloEphem> GloEphemPtr;  /* GLONASS星历智能指针 */

    /*
    GPS/Galileo/BeiDou星历结构体
    继承自EphemBase，包含这些系统共有的星历信息
    */
    struct Ephem : EphemBase
    {
        uint8_t sva;                         // 卫星精度指标 (URA index，消息中的sva字段)
        uint16_t code;                       // 编码标识 (GPS:L2码; GAL/BDS:数据源，消息中的code字段)
        uint16_t iodc;                       // 钟差数据版本号 (Issue of data, clock)

        double toc;                          // 钟差参考时间 (GPST，秒，消息中的toc字段)
        double toes;                      // 星历参考时刻的周内秒 
        double A, e, i0, omg, OMG0, M0;      // 卫星轨道参数
        double deln;                      // 平均角速度修正量 
        double OMGd;                      // 升交点赤经变化率
        double idot;                        // 轨道倾角变化率
        double cuc, cus, crc, crs, cic, cis; // 轨道摄动修正参数 (对应消息中同名字段)
        
        double f0, f1, f2;                // 卫星钟参数 (f0/f1/f2，对应消息中的f0/f1/f2字段)
        double tgd[2];                       // 群延迟参数
    };
    typedef std::shared_ptr<Ephem> EphemPtr;  /* GPS/Galileo/BeiDou星历智能指针 */

    /*
    观测数据结构体
    用于存储卫星观测数据
    */
    struct Obs {
        std::string prn;                // 卫星标识符，格式如 "G01"
        uint16_t week;                  // 整周数（不同系统基准不同）
        double tow;                     // 周内秒（北斗和GPS不同）
        std::vector<uint16_t> SNR;      // 信号强度，即信噪比
        std::vector<uint8_t> LLI;       // 失锁指示器，0 表示正常跟踪
        std::vector<std::string> code;  // 频率+调制方式，如 "1C"（2字符）
        std::vector<double> P;          // 伪距观测值 (m)
        std::vector<double> L;          // 载波相位观测值 (周)
        std::vector<double> D;          // 多普勒观测值 (Hz)（这个在读取时一定要注意！！GICI会自动将Hz转换为m/s，所以保持RINEX文件中的赫兹即可。但是GICI的自动处理不会取负！所以在读取文件赋值Doppler观测值时要注意取负！）

        //以上为消息中包含的变量，下面保留了一部分原来的结构体
        uint32_t sat;                   // 卫星编号
        gtime_t time;                   // GPST基准下的时间
        std::vector<double> freqs;      // 各信号对应的载波频率 (Hz)
        std::vector<double> P_std;      // 伪距观测值标准差 (m)
        std::vector<double> L_std;      // 载波相位观测值标准差 (周)
        std::vector<double> D_std;      // 多普勒观测值标准差 (Hz)
        std::vector<uint8_t> status;    // 观测状态标志
    };
    typedef std::shared_ptr<Obs> ObsPtr;  /* 观测数据智能指针 */

























}