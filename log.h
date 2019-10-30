/*
 *日志类头文件
 * */

#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

#define LOGINFO(...)        Logger::GetInstance().AddToQueue("INFO", __FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOGWARNING(...)     Logger::GetInstance().AddToQueue("WARNING", __FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)
#define LOGERROR(...)        Logger::GetInstance().AddToQueue("ERROR", __FILE__, __LINE__, __PRETTY_FUNCTION__, __VA_ARGS__)

//单例模式，以及nocopyable
class Logger {
public:
    static Logger& GetInstance();
    
    void SetFileName(const char* filename);

    bool Start();

    bool Stop();

    void AddToQueue(const char* pszLevel, const char* pszFile, int lineNo, const char* pszFuncSig, char* pszFmt, ...);
    
    ~Logger() {
        Stop();
    }
private:
    Logger() {
        Start();
    }

    Logger(const Logger& logger) = delete;
    Logger& operator=(const Logger& logger) = delete;

    void threadfunc();

private:
    std::string                     filename_;
    FILE*                           fp_{};
    std::shared_ptr<std::thread>    spthread_;
    std::mutex                      mutex_;
    std::condition_variable         cv_;            //有新的日志到来的标识
    bool                            exit_{false};
    std::list<std::string>          queue_;

};

#endif //__LOG_H__
