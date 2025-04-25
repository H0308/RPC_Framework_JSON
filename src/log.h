/*
    基于spdlog进行日志封装
    使用者可以按照如下方式使用：
    1. 自主决定日志等级
    2. 选择文件输出or控制台输出

    输出格式：
    [年-月-日 时:分:秒] [日志等级] [文件名:行号] 日志内容
    例子：
    [2025-04-25 22:25:25] [info] [test.cc:18] hello world
    使用方式：
    LOG(日志等级) << 日志内容 << 日志内容 ...
*/

#ifndef __rpc_log_h__
#define __rpc_log_h__

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"    // 文件日志
#include "spdlog/sinks/stdout_color_sinks.h" // 控制台彩色日志



#endif