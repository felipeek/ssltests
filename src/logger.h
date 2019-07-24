#ifndef RAWHTTPS_LOGGER_H
#define RAWHTTPS_LOGGER_H

enum Log_Level
{
    LOGGER_LOG_LEVEL_DEBUG = 0,
    LOGGER_LOG_LEVEL_INFO = 1,
    LOGGER_LOG_LEVEL_WARNING = 2,
    LOGGER_LOG_LEVEL_ERROR = 3
};

// core functions
void rawhttps_logger_level_set(enum Log_Level level);
void rawhttps_logger_log_debug(const char* format, ...);
void rawhttps_logger_log_info(const char* format, ...);
void rawhttps_logger_log_warning(const char* format, ...);
void rawhttps_logger_log_error(const char* format, ...);
#endif