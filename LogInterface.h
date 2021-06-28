/*
Copyright 2019, orcaer@yeah.net  All rights reserved.

Author: orcaer@yeah.net

Last modified: 2019-2-28

Description: https://github.com/wlgq2/eomaia
*/

#ifndef     LOG_INTERFACE_H
#define     LOG_INTERFACE_H

#include <string>
#include <functional>
#include <atomic>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

//创建目录所需要包含的头文件
#include <direct.h>
#include <io.h>

namespace base
{

class LogInterface
{
public:
    virtual ~LogInterface(){}

    virtual void warn(const std::string& data) = 0;
    virtual void error(const std::string& data) = 0;
    virtual void info(const std::string& data) = 0;
    virtual void debug(const std::string& data) = 0;
};

class Log : public LogInterface
{
public:
    using OutMessageType = std::function<void(const std::string&)>;
    enum Level{
        Debug = 0,
        Info,
        Warn,
        Error
    };
    static Log* Instance()
    {
        static Log single;
        return &single;
    }

    void registerInterface(LogInterface* handle)
    {
        handle_ = handle;
        funcs_[Debug] = std::bind(&LogInterface::debug, handle_, std::placeholders::_1);
        funcs_[Info] = std::bind(&LogInterface::info, handle_, std::placeholders::_1);
        funcs_[Warn] = std::bind(&LogInterface::warn, handle_, std::placeholders::_1);
        funcs_[Error] = std::bind(&LogInterface::error, handle_, std::placeholders::_1);
    }
    void write(int level,std::string& data)
    {
        if ((handle_) &&(level<= Error) && (level >= Debug))
        {
            funcs_[level](data);
        }
        else
            std::cout<<data;
    }
    void write(int level, std::string&& data)
    {
        if ((handle_) && (level <= Error))
        {
            funcs_[level](data);
        }
        else
            std::cout<<data;
    }
    void warn(const std::string& data)
    {
        if (handle_) {
            //handle_->warn(data);
            write_to_log_file(data);
        }
        else
            std::cout<<data<<std::endl;
    }
    void warn(const std::string&& data)
    {
        if (handle_)
            handle_->warn(data);
        else
            std::cout<<data<<std::endl;
    }
    void error(const std::string& data)
    {
        if (handle_) {
            //handle_->error(data);
            write_to_log_file(data);
        }
        else
            std::cerr<<data<<std::endl;
    }
    void error(const std::string&& data)
    {
        if (handle_) {
            handle_->error(data);
        }
        else
            std::cerr<<data<<std::endl;
    }
    void info(const std::string& data)
    {
        if (handle_) {
            //handle_->info(data);
            write_to_log_file(data);
        }
        else
            std::cout<<data<<std::endl;
    }
    void info(const std::string&& data)
    {
        if (handle_)
            handle_->info(data);
        else
            std::cout<<data<<std::endl;
    }
    void debug(const std::string& data)
    {
        if (handle_) {
            //handle_->debug(data);
            write_to_log_file(data);
        }
        else
            std::cout<<data<<std::endl;
    }
    void debug(const std::string&& data)
    {
        if (handle_)
            handle_->debug(data);
        else
            std::cout<<data<<std::endl;

    }

    static void ToHex(std::string& message,const char* data,unsigned int size)
    {
        for (unsigned int i = 0; i < size; i++)
        {
            char buf[8];
            std::sprintf(buf, " 0x%x ", (unsigned char)data[i]);
            message += buf;
        }
    }

    //接收不同类型的“<<”符重载
    template<typename T>
    Log& operator << (T&& param) {
        std::string str_data;
        std::stringstream stream;
        stream << param;
        stream >> str_data;
        write(currentLevel_, str_data);
        return *Instance();
    }

    template<typename T>
    Log& operator << (T& param) {
        std::string str_data;
        std::stringstream stream;
        stream << param;
        stream >> str_data;
        write(currentLevel_, str_data);
        return *Instance();
    }


    static Log& OUT(Level level)
    {
        Instance()->currentLevel_ = level;
        return *Instance();
    }
private:
    Log():handle_(nullptr)
    {
    }

    //根据Level转换为对应的字符串
    std::string transfrom_to_str(const Level& level) {
        switch (level) {
        case Debug:
            return "Debug";
        case Info:
            return "Info";
        case Warn:
            return "Warn";
        case Error:
            return "Error";
        default:
            return "Unknow";
        }
    }

    //根据当前的时间将时间字符串切割（这个用于log日志的文件名）
    std::string slice_time() {
        time_t tm = time(0);
        std::string str = ctime(&tm);
        for (auto it = str.begin(); it != str.end(); ) {
            if ((*it) == ' ') {
                *it = '_';
                ++it;
            }
            else if ((*it) == '\n') {
                it = str.erase(it);
            }
            else {
                ++it;
            }
        }
        return std::move(str);
    }
    //日志文件的一套封装
    void write_to_log_file(const std::string& data) {
        std::string str = slice_time();
        std::string strBegin = str.substr(0, 10);
        std::string strEnd;
        auto pos = str.find_last_of('_');
        if (pos != std::string::npos) {
            strEnd = str.substr(pos);
        }

        std::string dir;
        std::string path;

        //指针p用于判断当前系统是32位还是64位
        void* p = nullptr;
        //debug模式和release模式下路径不一样，所以这里分开处理
#ifdef _DEBUG 
        if (sizeof(p) == 4) {
            dir = "./x86/Debug/log/";
        }
        else {
            dir = "./x64/Debug/log/";
        }
            path = dir + "[" + transfrom_to_str(currentLevel_) + "]" + strBegin + strEnd + ".log";
#else
        if (sizeof(p) == 4) {
            dir = "./x86/Release/log/";
        }
        else {
            dir = "./x64/Release/log/";
        }
        path = dir + "[" + transfrom_to_str(currentLevel_) + "]" + strBegin + strEnd + ".log";
#endif
        //如果目录不存在，则创建该目录
        char dir_tmp[256] = { 0 };
        for (int i = 0; i < dir.size(); i++) {
            dir_tmp[i] = dir[i];
            if (dir_tmp[i] == '\\' || dir_tmp[i] == '/') {
                if (_access(dir_tmp, 0) == -1) {
                    int ret = _mkdir(dir_tmp);
                    if (ret == -1) {
                        return;
                    }
                }
            }
        }
        file_.open(path, std::ios::app | std::ios::in | std::ios::out);
        if (!file_.is_open()) {
            std::cerr << "文件打开失败" << std::endl;
            return;
        }
        file_ << "[" << str << " " << transfrom_to_str(currentLevel_) << "]:" << data << std::endl;
        file_.close();
        //handle_->warn(data);
    }

    LogInterface* handle_;
    OutMessageType funcs_[Error + 1];

    std::atomic<Level> currentLevel_;

    std::fstream file_;
};
}
#endif // !  UV_LOG_INTERFACE_H

