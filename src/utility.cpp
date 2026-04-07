#include "constant.hpp"
#include "utility.hpp"
using namespace std;
namespace gici_datapacker{

    uint32_t satsys(uint32_t sat, uint32_t *prn)
    {
        uint32_t sys = SYS_NONE;
        if (sat <= 0 || sat > MAX_SAT) sat = 0;
        else if (sat <= N_SAT_GPS) {
            sys = SYS_GPS; sat += MIN_PRN_GPS-1;
        }
        else if ((sat-=N_SAT_GPS) <= N_SAT_GLO) {
            sys = SYS_GLO; sat += MIN_PRN_GLO-1;
        }
        else if ((sat-=N_SAT_GLO) <= N_SAT_GAL) {
            sys = SYS_GAL; sat += MIN_PRN_GAL-1;
        }
        else if ((sat-=N_SAT_GAL) <= N_SAT_BDS) {
            sys = SYS_BDS; sat += MIN_PRN_BDS-1; 
        }
        else sat = 0;
        if (prn) *prn = sat;
        return sys;
    }
    std::string sat2str(uint32_t sat_no)
    {
        std::stringstream ss;
        uint32_t prn = 0;
        uint32_t sys = satsys(sat_no, &prn);
        switch (sys)
        {
            case SYS_GPS:
                ss << "G" << std::setw(2) << std::setfill('0') << prn;
                break;
            case SYS_GLO:
                ss << "R" << std::setw(2) << std::setfill('0') << prn;
                break;
            case SYS_BDS:
                ss << "C" << std::setw(2) << std::setfill('0') << prn;
                break;
            case SYS_GAL:
                ss << "E" << std::setw(2) << std::setfill('0') << prn;
                break;
            case SYS_SBS:
                ss << "S" << std::setw(2) << std::setfill('0') << prn;
                break;
            case SYS_QZS:
                ss << "J" << std::setw(2) << std::setfill('0') << prn;
                break;
            default:
                LOG(WARNING) << "currently not support satelite system id " << sys;
                break;
        }
        return ss.str();
    }
    uint32_t sat_no(uint32_t sys, uint32_t prn)
    {
        if (prn == 0) return 0;
        switch (sys) {
            case SYS_GPS:
                if (prn < MIN_PRN_GPS || prn > MAX_PRN_GPS) return 0;
                return prn-MIN_PRN_GPS+1;
            case SYS_GLO:
                if (prn < MIN_PRN_GLO || prn > MAX_PRN_GLO) return 0;
                return N_SAT_GPS+prn-MIN_PRN_GLO+1;
            case SYS_GAL:
                if (prn < MIN_PRN_GAL || prn > MAX_PRN_GAL) return 0;
                return N_SAT_GPS+N_SAT_GLO+prn-MIN_PRN_GAL+1;
            case SYS_BDS:
                if (prn < MIN_PRN_BDS || prn > MAX_PRN_BDS) return 0;
                return N_SAT_GPS+N_SAT_GLO+N_SAT_GAL+prn-MIN_PRN_BDS+1;
        }
        return 0;
    }
    uint32_t str2sat(const std::string &sat_str){
        if (sat_str.size() < 3)
        {
            LOG(ERROR) << "错误的卫星标记，字符串长度<3" << sat_str;
            return 0;
        }
        const uint32_t prn = std::stoul(sat_str.substr(1, 2));//提取字符串后两位数字
        switch (sat_str.at(0))
        {
            case 'G':
                return sat_no(SYS_GPS, prn);
            case 'R':
                return sat_no(SYS_GLO, prn);
            case 'C':
                return sat_no(SYS_BDS, prn);
            case 'E':
                return sat_no(SYS_GAL, prn);
            case 'S':
                return sat_no(SYS_SBS, prn);
            case 'J':
                return sat_no(SYS_QZS, prn);
        }
        return 0;
    }

    gtime_t time_add(gtime_t t, double sec)
    {
        t.sec += sec;
        double tt = floor(t.sec);
        t.time += static_cast<int>(tt);
        t.sec -= tt;
        return t;
    }
    double time_diff(gtime_t t1, gtime_t t2)
    {
        return difftime(t1.time,t2.time)+t1.sec-t2.sec;
    }
    gtime_t gpst2utc(gtime_t t)
    {
        return time_add(t, -LEAP_SECONDS); // UTC = GPST-18
    }
    gtime_t utc2gpst(gtime_t t)
    {
        return time_add(t, LEAP_SECONDS); // GPST = UTC+18
    }
    gtime_t gpst2bdt (gtime_t t){
        return time_add(t, -14.0); //BDT=GPST-14
    }
    gtime_t bdt2gpst (gtime_t t){
        return time_add(t, 14.0); //GPST=BDT+14
    }

    void gpst2bdt (int &week, double &tow){
        week-=1356;//北斗周=GPS周-1356
        tow-=14;//北斗周内秒=GPS周内秒-14
    }
    void bdt2gpst (int &week, double &tow){
        week+=1356;//GPS周=北斗周+1356
        tow+=14;//GPS周内秒=北斗周内秒+14
    }

    double gpst2bdt_week_tow(gtime_t t, int *week){
        double tow = gpst2gpst_week_tow(t, week); //先得到GPST基准下的周和周内秒
        gpst2bdt(*week,tow); // 其实是因为北斗和GPS相差14秒，不方便直接计算。所以才调用函数进行。
        return tow;
    }
    double gpst2gpst_week_tow(gtime_t t, int *week){
        gtime_t t0 = epoch_array2gpst(gpst0);//GPST的时间起点

        time_t sec = t.time - t0.time;
        uint32_t w = (uint32_t)(sec / (86400*7));
        if (week) *week = w;
        return (double)(sec - w*86400*7) + t.sec;
    }
    double gpst2galileo_week_tow(gtime_t t, int *week){
        gtime_t t0 = epoch_array2gpst(gst0);//Galileo的时间起点

        time_t sec = t.time - t0.time;
        uint32_t w = (uint32_t)(sec / (86400*7));
        if (week) *week = w;
        return (double)(sec - w*86400*7) + t.sec;
    }
    
    void gpst2epoch_array(gtime_t t, double *ep)
    {
        const int mday[] = { /* # of days in a month */
            31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
            31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
        };
        int days, sec, mon, day;
        
        days = (int)(t.time / 86400);
        sec = (int)(t.time - (time_t)days*86400);
        days+=3657;//转换成以1970年1月1日为基准，方便适配下面的年月日转换
        for (day=days%1461, mon=0; mon<48; mon++) {
            if (day >= mday[mon]) day -= mday[mon]; else break;
        }
        ep[0] = 1970 + days/1461*4 + mon/12;
        ep[1] = mon%12 + 1;
        ep[2] = day + 1;
        ep[3] = sec / 3600;
        ep[4] = sec%3600 / 60;
        ep[5] = sec%60 + t.sec;
    }
    gtime_t epoch_array2gpst(const double *ep)
    {
        const int doy[] = {1,32,60,91,121,152,182,213,244,274,305,335};
        gtime_t time = {0};
        int days, sec, year=(int)ep[0], mon=(int)ep[1], day=(int)ep[2];
        
        if (year < 1970 || year > 2099 || mon < 1 || mon > 12) return time;
        
        days = (year-1970)*365 + (year-1969)/4 + doy[mon-1] + day-2 + (year%4==0&&mon>=3?1:0)
            -3657; // 减去3657，转换成以1980年1月6日为基准
        sec = (int)floor(ep[5]);
        time.time = (time_t)days*86400 + (int)ep[3]*3600 + (int)ep[4]*60 + sec;
        time.sec = ep[5] - sec;
        return time;
    }

    gtime_t gpst2unix (gtime_t t){
        gtime_t result;
        result.time = t.time+315964800.0-LEAP_SECONDS; //TODO: 这里可能是错的
        result.sec = t.sec;
        return result;
    }


    GnssObservations obs2msg(const vector<ObsPtr>& obs_list){
        GnssObservations obs_msgs;
        for(auto obs:obs_list){
            GnssObservation obs_msg;
            obs_msg.prn=obs->prn;
            obs_msg.week=obs->week;
            obs_msg.tow=obs->tow;
            obs_msg.code=obs->code;
            obs_msg.D=obs->D;
            obs_msg.P=obs->P;
            obs_msg.L=obs->L;
            obs_msg.SNR = obs->SNR;
            obs_msg.LLI=obs->LLI;
            obs_msgs.observations.push_back(obs_msg);
        }
        gtime_t unix_t = gpst2unix(obs_list[0]->time);
       obs_msgs.header.stamp = ros::Time(
            unix_t.time,  // 整数秒
            static_cast<uint32_t>(std::round(unix_t.sec * 1e9))  // 小数秒转纳秒
        );
        obs_msgs.header.frame_id = "gnss";
        return obs_msgs;
    }
    GnssEphemeris ephem2msg(EphemPtr ephem) {
        GnssEphemeris msg;
        msg.prn   = ephem->prn;
        msg.week  = ephem->week;
        msg.sva   = ephem->sva;
        msg.code  = ephem->code;
        msg.iode  = ephem->iode;
        msg.iodc  = ephem->iodc;
        msg.svh   = ephem->svh;
        msg.toc   = ephem->toc;
        msg.idot  = ephem->idot;
        msg.crs   = ephem->crs;
        msg.deln  = ephem->deln;
        msg.M0    = ephem->M0;
        msg.cuc   = ephem->cuc;
        msg.e     = ephem->e;
        msg.cus   = ephem->cus;
        msg.A     = ephem->A;
        msg.toes  = ephem->toes;
        msg.cic   = ephem->cic;
        msg.OMG0  = ephem->OMG0;
        msg.cis   = ephem->cis;
        msg.i0    = ephem->i0;
        msg.crc   = ephem->crc;
        msg.omg   = ephem->omg;
        msg.OMGd  = ephem->OMGd;
        msg.tgd.resize(2);
        msg.tgd[0] = ephem->tgd[0];
        msg.tgd[1] = ephem->tgd[1];
        msg.f2    = ephem->f2;
        msg.f1    = ephem->f1;
        msg.f0    = ephem->f0;
        return msg;
    }
    GlonassEphemeris glo_ephem2msg(GloEphemPtr ephem) {
        GlonassEphemeris msg;
        msg.prn   = ephem->prn;
        msg.week  = ephem->week;
        msg.frq   = ephem->frq;
        msg.iode  = ephem->iode;
        msg.svh   = ephem->svh;
        msg.age   = ephem->age;
        msg.toe   = ephem->toe;
        msg.tof   = ephem->tof;
        
        msg.pos.resize(3);
        msg.vel.resize(3);
        msg.acc.resize(3);
        for (int i = 0; i < 3; ++i) {
            msg.pos[i] = ephem->pos[i];
            msg.vel[i] = ephem->vel[i];
            msg.acc[i] = ephem->acc[i];
        }
        
        msg.taun  = ephem->taun;
        msg.gamn  = ephem->gamn;
        msg.dtaun = ephem->dtaun;
        return msg;
    }



    void FindNewEphem(ObsPtr& obs,vector<EphemBasePtr>& ephems,EphemBasePtr& resultEphem){
        double min_t = 9999999.0;
        for (auto e : ephems) {
            double dt = time_diff(obs->time, e->header_t); // obs - header_t
            if (dt > 0.0 && dt < min_t) { // 过去的最近，也就是星历更早发布，并且距离最小
                min_t = dt;
                resultEphem = e;
            }
        }
    }




}