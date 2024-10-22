//
// Created by rui836662398 on 2021/2/9.
//

#ifndef MYWEBSERVER_LOGGING_H
#define MYWEBSERVER_LOGGING_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

const char* strerror_tl(int saved_errno);

#define TRACE(x) spdlog::get("log")->trace("[{}]{}", unit_name, x)
#define DEBUG(x) spdlog::get("log")->debug("[{}]{}", unit_name, x)
#define INFO(x) spdlog::get("log")->info("[{}]{}", unit_name, x)
#define ERROR(x) spdlog::get("log")->error("[{}]{}", unit_name, x)
#define FATAL(x) spdlog::get("log")->error("[{}]{}.System exit.", unit_name, x),abort()

#define ERROR_P(x) spdlog::get("log")->error("[{}]{} error - {}", unit_name, x, strerror_tl(errno))
#define FATAL_P(x) spdlog::get("log")->error("[{}]{} error - {}.System exit.", unit_name, x, strerror_tl(errno)),abort()



class LOG {
   public:
    // singleton
    static LOG& GetInstance(){
        static LOG log;
        return log;
    }

    ~LOG() {
        spdlog::get("log")->flush();
    }

   private:
    //配置日志文件、写入模式
    LOG() {
        //日志实例名、日志文件最大大小、滚动文件数量（日志太多的时候，当前文件重命名_1,_2,_3.再写新的文件）
        auto logger = spdlog::rotating_logger_mt("log", "log/log.txt", 5 * 1024 * 1024, 10);
        //格式设置：[年月日 时分秒毫秒] [日志等级缩写] 日志正文
        logger->set_pattern("[%Y-%m-%d %T.%e] [%l] %v");
        //遇到info级别，立马将缓存的buffer写到文件中
        spdlog::flush_on(spdlog::level::info);
        spdlog::get("log")->set_level(spdlog::level::debug);
    }
};

static LOG& l = LOG::GetInstance();

#endif  // MYWEBSERVER_LOGGING_H
