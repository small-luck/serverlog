/*
 *日志类实现文件
 * */
#include "log.h"
#include <time.h>
#include <stdio.h>
#include <memory>
#include <stdarg.h>

Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

void Logger::SetFileName(const char* filename)
{
    filename_ = filename;
}

bool Logger::Start()
{
    if (filename_.empty()) {
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        char timestr[64] = {0};
        sprintf(timestr, "%04d%02d%02d%02d%02d%02d.imserver.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        filename_ = timestr;
    }

    fp_ = fopen(filename_.c_str(), "wt+");
    if (fp_ == NULL)
        return false;

    //启动写线程
    spthread_.reset(new std::thread(std::bind(&Logger::threadfunc, this)));

    return true;
}

bool Logger::Stop()
{
    exit_ = true;
    cv_.notify_one();

    //等待写线程结束
    spthread_->join();

    fclose(fp_);

    return true;
}

void Logger::AddToQueue(const char* pszLevel, const char* pszFile, int lineNo, const char* pszFuncsig, char*pszFmt, ...)
{
    char msg[256] = {0};

    va_list vArgList;
    va_start(vArgList, pszFmt);
    vsnprintf(msg, 256, pszFmt, vArgList);
    va_end(vArgList);

    time_t now = time(NULL);
    struct tm* tmstr = localtime(&now);
    char content[512] = {0};
    sprintf(content, "[%04d-%02d-%02d %02d:%02d:%02d][%s][%04x][%s:%d %s] %s\n",
                tmstr->tm_year + 1900,
                tmstr->tm_mon + 1,
                tmstr->tm_mday,
                tmstr->tm_hour,
                tmstr->tm_min,
                tmstr->tm_sec,
                pszLevel,
                std::this_thread::get_id(),
                pszFile,
                lineNo,
                pszFuncsig,
                msg);
    {
        std::lock_guard<std::mutex> guard(mutex_);
        queue_.emplace_back(content);
    }

    cv_.notify_one();
}

void Logger::threadfunc()
{
    if (fp_ == NULL)
        return;

    while (!exit_) {
        //写日志
        std::unique_lock<std::mutex> guard(mutex_);
        while (queue_.empty()) {
            if (exit_) 
                return;

            cv_.wait(guard);
        }

        //写日志
        const std::string& str = queue_.front();

        fwrite((void*)str.c_str(), str.length(), 1, fp_);
        fflush(fp_);

        queue_.pop_front();
    }
}
