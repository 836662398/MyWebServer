//
// Created by rui836662398 on 2021/2/9.
//

#include "logging.h"

#include <spdlog/sinks/rotating_file_sink.h>

class LOG {
   public:
    //配置日志文件、写入模式
    LOG() {
        //日志实例名、日志文件最大大小、滚动文件数量（日志太多的时候，当前文件重命名_1,_2,_3.再写新的文件）
        auto logger = spdlog::rotating_logger_mt("log", "log/log.txt", 2 * 1024 * 1024, 3);
        //格式设置：[年月日 时分秒毫秒] [日志等级缩写] 日志正文
        logger->set_pattern("[%Y-%m-%d %T.%e] [%L] %v");
        //遇到info级别，立马将缓存的buffer写到文件中
        spdlog::flush_on(spdlog::level::info);
        spdlog::get("log")->set_level(spdlog::level::debug);
    }

    ~LOG() {
        spdlog::get("log")->flush();
    }
};

static LOG logger;