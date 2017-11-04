#include <log4cplus/loggingmacros.h>
#include <log4cplus/logger.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/configurator.h>
//使用日志配置初始化log日志
#define INIT_LOG(filePath) log4cplus::PropertyConfigurator::doConfigure(filePath)

//设置日志级别
#define SET_LOG_LEVEL(level) log4cplus::Logger::getRoot().setLogLevel(level)

//定义日志输出，日志级别由低到高
#define LOG_TRACE(logs) LOG4CPLUS_TRACE(log4cplus::Logger::getRoot(), logs)
#define LOG_DEBUG(event) LOG4CPLUS_DEBUG(log4cplus::Logger::getRoot(), event)
#define LOG_INFO(logs) LOG4CPLUS_INFO(log4cplus::Logger::getRoot(), logs)
#define LOG_WARN(logs) LOG4CPLUS_WARN(log4cplus::Logger::getRoot(), logs)
#define LOG_ERROR(logs) LOG4CPLUS_ERROR(log4cplus::Logger::getRoot(), logs)
#define LOG_FATAL(logs) LOG4CPLUS_FATAL(log4cplus::Logger::getRoot(), logs)

