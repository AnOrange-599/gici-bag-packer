#include "rinex_helper.hpp"
using namespace std;


namespace gici_datapacker{

    map<char, vector<string>> readRinexObsHeader(const string& path)
    {
        map<char, vector<string>> sys2type; // 每个卫星系统对应的观测值列表
        char sys_char = ' ';

        ifstream rinex_file; rinex_file.open(path); //打开文件
        string line;
        while (getline(rinex_file, line))
        {
            if (line.find("RINEX VERSION / TYPE") != string::npos && line.find("3.04") == string::npos)
            {
                LOG(ERROR) << "Only RINEX 3.04 is supported for observation file";
                return {};//字典不能无值返回
            }
            else if (line.find("SYS / # / OBS TYPES") != string::npos)
            {
                if (line.at(0) != ' ')
                {
                    sys_char = line.at(0);
                    sys2type.emplace(sys_char, vector<string>());
                }
                for (size_t i = 0; i < 13; ++i)
                    if (line.substr(7+4*i, 3) != "   ")
                        sys2type.at(sys_char).emplace_back(line.substr(7+4*i, 3));
            }
            else if (line.find("END OF HEADER") != string::npos)  break;
        }
        return sys2type;
    }
    
    gtime_t rinexEphocLine2Ephoc(const string ephoc_str, int &num_obs)
    {
        LOG_IF(FATAL, ephoc_str.at(0) != '>') << "Invalid Observation record " << ephoc_str;
        LOG_IF(FATAL, ephoc_str.at(31) != '0') << "Invalid Epoch data " << ephoc_str.at(31)-48;
        std::vector<double> epoch_time;
        epoch_time.emplace_back(std::stod(ephoc_str.substr(2, 4)));
        epoch_time.emplace_back(std::stod(ephoc_str.substr(7, 2)));
        epoch_time.emplace_back(std::stod(ephoc_str.substr(10, 2)));
        epoch_time.emplace_back(std::stod(ephoc_str.substr(13, 2)));
        epoch_time.emplace_back(std::stod(ephoc_str.substr(16, 2)));
        epoch_time.emplace_back(std::stod(ephoc_str.substr(18, 11)));
        gtime_t obs_time = epoch_array2gpst(&(epoch_time[0]));
        num_obs = std::stoi(ephoc_str.substr(32, 3));//这一历元的观测值个数
        return obs_time;
    }
    
    
    ObsPtr rinexObsLine2Obs(const string rinex_str, const map<char, vector<string>> &sys2type)
    {
        ObsPtr obs = std::make_shared<Obs>(); // 指针一定要记得初始化，分配内存
        char sys_char = rinex_str.at(0);
        if (char2sys.count(sys_char) == 0)   return obs; // 如果是GCER之外的，直接返回空观测值。所以加入列表时，需要过滤掉空观测值
        obs->prn = rinex_str.substr(0,3); // G02
        obs->sat = str2sat(obs->prn); // 构造卫星编号
        vector<string> data_types = sys2type.at(sys_char);//比如GPS对应C1C L1C D1C S1C C2S L2S D2S S2S
        uint32_t obs_line_offset = 3; // 4个观测值的偏移量
        for(int i=0;i<data_types.size()/4;i++){ // 存在8个观测值类型，即2个频率
            string long_field_str = rinex_str.substr(obs_line_offset, 62);// 该频率的4个观测值对应的字符串
            if (long_field_str.find_first_not_of(' ') == string::npos){ //除了空格没有别的字符
                continue;//4个空值直接跳过
            }
            for(int j=0;j<4;j++){//遍历该频率下每一个观测值
                string field_str = rinex_str.substr(obs_line_offset, 14);
                double field_value = stod(field_str);//得到观测值的double型
                //填入code
                string code = data_types[4*i + j].substr(1,2);
                if (std::find(obs->code.begin(), obs->code.end(), code) == obs->code.end()){ // 如果没有找到，说明还没有添加
                    obs->code.push_back(code);
                }
                // 填入具体的观测值
                if(data_types[4*i + j].at(0)=='L') // 载波观测值
                    obs->L.emplace_back(field_value);
                else if(data_types[4*i + j].at(0)=='C') // 伪距
                    obs->P.emplace_back(field_value);
                else if(data_types[4*i + j].at(0)=='D') // 多普勒
                    obs->D.emplace_back(-field_value);
                else if(data_types[4*i + j].at(0)=='S') // 信噪比
                    obs->SNR.emplace_back(static_cast<uint16_t>(field_value * 1000)); // 注意单位1000倍
                else // 不属于4类观测值
                    LOG(FATAL) << "未知的观测值类型：" << data_types[4*i + j].at(0);
                obs_line_offset += 14 + 2; // 下一观测值的初始偏移量
            }
            obs->LLI.emplace_back(0); // 失锁指示器赋值0
        }
        return obs;
    }


    void rinex2ephems(const std::string &rinex_filepath, std::map<uint32_t, std::vector<EphemBasePtr>> &sat2ephem)
    {
        std::ifstream ephem_file(rinex_filepath);
        std::string line;
        // 读取文件头
        while(std::getline(ephem_file, line))
        {
            if (line.find("RINEX VERSION / TYPE") != std::string::npos && line.find("3.04") == std::string::npos)//判断条件：包含版本号，同时不包含3.04，说明版本错误
            {
                LOG(ERROR) << "Only RINEX 3.04 is supported for observation file";
                return;
            }
            if (line.find("BDUT") != std::string::npos) // 包含了BDUT参数，说明使用原生时间系统
                use_native_time = true;
            if (line.find("END OF HEADER") != std::string::npos)
                break;
        }
        // 读取文件体
        while(std::getline(ephem_file, line))
        {
            if (line.at(0) == 'G' || line.at(0) == 'C' || line.at(0) == 'E')
            {
                std::vector<std::string> ephem_lines;
                ephem_lines.push_back(line);
                for (size_t i = 0; i < 7; ++i)
                {
                    std::getline(ephem_file, line);
                    ephem_lines.push_back(line);
                }
                EphemPtr ephem = rinex_line2ephem(ephem_lines);
                if (!ephem)  continue;
                if (sat2ephem.count(ephem->sat) == 0)
                    sat2ephem.emplace(ephem->sat, std::vector<EphemBasePtr>());
                sat2ephem.at(ephem->sat).push_back(ephem);
            }
            else if (line.at(0) == 'R')
            {
                std::vector<std::string> ephem_lines;
                ephem_lines.push_back(line);
                for (size_t i = 0; i < 3; ++i)
                {
                    std::getline(ephem_file, line);
                    ephem_lines.push_back(line);
                }
                GloEphemPtr glo_ephem = rinex_line2glo_ephem(ephem_lines);
                if(!glo_ephem) continue;
                if (sat2ephem.count(glo_ephem->sat) == 0)
                    sat2ephem.emplace(glo_ephem->sat, std::vector<EphemBasePtr>());
                sat2ephem.at(glo_ephem->sat).push_back(glo_ephem);
            }
        }
    }

    GloEphemPtr rinex_line2glo_ephem(std::vector<std::string> ephem_lines) {
        GloEphemPtr ephem = make_shared<GloEphem>();

        string line = ephem_lines[0];
        ephem->sat = str2sat(line.substr(0,3));
        ephem->prn = line.substr(0,3);
        string time_str = line.substr(4, 19);               //TODO：Toc时间赋值给Toe
        double arr[6]{}; int i=0; 
        for(stringstream ss(time_str); ss>>arr[i] && i<6; i++); // 因为以空格分割，有天生优势。
        gtime_t ephoc_t = epoch_array2gpst(&arr[0]); // 将'年月日时分秒'转换为gtime_t（我希望这里的时间基准是GPST）
        int week;
        ephem->toe = gpst2gpst_week_tow(ephoc_t, &week);
        ephem->week = week;                                 // 时间赋值完成
        ephem->header_t = ephoc_t;

        ephem->taun = -stod(line.substr(23, 19));
        ephem->gamn = stod(line.substr(42, 19));
        ephem->iode = 29;

        line = ephem_lines[1];
        ephem->pos[0] = stod(line.substr(4, 19)) * 1000;
        ephem->vel[0] = stod(line.substr(23, 19)) * 1000;
        ephem->acc[0] = stod(line.substr(42, 19)) * 1000;
        ephem->svh = static_cast<uint8_t>(stoi(line.substr(61, 19)));

        line = ephem_lines[2];
        ephem->pos[1] = stod(line.substr(4, 19)) * 1000;
        ephem->vel[1] = stod(line.substr(23, 19)) * 1000;
        ephem->acc[1] = stod(line.substr(42, 19)) * 1000;
        ephem->frq = static_cast<uint8_t>(stoi(line.substr(61, 19)));

        line = ephem_lines[3];
        ephem->pos[2] = stod(line.substr(4, 19)) * 1000;
        ephem->vel[2] = stod(line.substr(23, 19)) * 1000;
        ephem->acc[2] = stod(line.substr(42, 19)) * 1000;
        ephem->age = static_cast<uint8_t>(stoi(line.substr(61, 19)));

        // if(use_native_time == true) // 如果使用的时间系统是UTC
            ephem->toe += LEAP_SECONDS; // 上面错误地将时间戳转换为GPST，GPST-18=UTC。这里需要补上18秒，是加！！
        ephem->tof = 0.0; // Python代码中是这样处理的
        return ephem;
    }

    EphemPtr rinex_line2ephem(std::vector<std::string> ephem_lines) {
        EphemPtr ephem = make_shared<Ephem>();
        std::string line = ephem_lines[0];
        char sys_char = line[0];
        ephem->sat = str2sat(line.substr(0,3));

        // 处理第1行
        ephem->prn = line.substr(0,3);
        string time_str = line.substr(4, 19); //Toc时间赋值
        double arr[6]{}; int i=0; 
        for(stringstream ss(time_str); ss>>arr[i] && i<6; i++);
        gtime_t ephoc_t = epoch_array2gpst(&arr[0]); // 将'年月日时分秒'转换为gtime_t（我希望这里的时间基准是GPST）
        int week;
        ephem->toc = gpst2gpst_week_tow(ephoc_t, &week);
        ephem->week = week;
        if(sys_char=='C'){ephem->toc-=14;ephem->week-=1356;}
        ephem->header_t = ephoc_t;

        ephem->f0 = std::stof(line.substr(23, 19));
        ephem->f1 = std::stof(line.substr(42, 19));
        ephem->f2 = std::stof(line.substr(61, 19));
        
        line = ephem_lines[1]; // 处理第2行
        ephem->iode = static_cast<uint8_t>(std::stoi(line.substr(4, 19)));
        ephem->crs = std::stof(line.substr(23, 19));
        ephem->deln = std::stof(line.substr(42, 19));
        ephem->M0 = std::stof(line.substr(61, 19));
        
        line = ephem_lines[2]; // 第3行
        ephem->cuc = std::stof(line.substr(4, 19));
        ephem->e = std::stof(line.substr(23, 19));
        ephem->cus = std::stof(line.substr(42, 19));
        ephem->A = pow(std::stof(line.substr(61, 19)), 2); // 注意平方
        
        line = ephem_lines[3]; // 第4行
        ephem->toes = std::stof(line.substr(4, 19));
        ephem->cic = std::stof(line.substr(23, 19));
        ephem->OMG0 = std::stof(line.substr(42, 19));
        ephem->cis = std::stof(line.substr(61, 19));
        // if (sys_char == 'C' && use_native_time == false) // 如果使用的是GPST基准，需要转换！
        //     ephem->toes -= 14; // 周内秒，从GPST基准，转到北斗基准
        
        line = ephem_lines[4]; // 第5行
        ephem->i0 = std::stof(line.substr(4, 19));
        ephem->crc = std::stof(line.substr(23, 19));
        ephem->omg = std::stof(line.substr(42, 19));
        ephem->OMGd = std::stof(line.substr(61, 19));
        
        // 第6行之后，每个系统单独处理
        if (sys_char == 'G') {
            line = ephem_lines[5]; // 第6行
            ephem->idot = std::stof(line.substr(4, 19));
            ephem->code = 1;
            line = ephem_lines[6]; // 第7行
            ephem->sva = static_cast<uint8_t>(std::stoi(line.substr(4, 19)));
            ephem->svh = static_cast<uint8_t>(std::stoi(line.substr(23, 19)));
            ephem->tgd[0] = std::stof(line.substr(42, 19));
            ephem->iodc = static_cast<uint16_t>(std::stoi(line.substr(61, 19)));
        }
        else if (sys_char == 'C') {
            line = ephem_lines[5]; // 第6行
            ephem->idot = std::stof(line.substr(4, 19));
            line = ephem_lines[6]; // 第7行
            ephem->sva = static_cast<uint8_t>(std::stoi(line.substr(4, 19)));
            ephem->svh = static_cast<uint8_t>(std::stoi(line.substr(23, 19)));
            ephem->tgd[0] = std::stof(line.substr(42, 19));
            ephem->tgd[1] = std::stof(line.substr(61, 19));
            line = ephem_lines[7]; // 第8行
            ephem->iodc = static_cast<uint16_t>(std::stoi(line.substr(23, 19)));
        }
        else if (sys_char == 'E') {
            line = ephem_lines[5]; // 第6行
            ephem->idot = std::stof(line.substr(4, 19));
            ephem->code = 516;
            line = ephem_lines[6]; // 第7行
            ephem->sva = static_cast<uint8_t>(std::stoi(line.substr(4, 19)));
            ephem->svh = static_cast<uint8_t>(std::stoi(line.substr(23, 19)));
            ephem->tgd[0] = std::stof(line.substr(42, 19));
            ephem->tgd[1] = std::stof(line.substr(61, 19));
        }

        return ephem; // 补充返回语句
    }

}