#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static enum Log_Level log_level = LOGGER_LOG_LEVEL_ERROR;

static void logger_log_out(FILE* target, const char* level, const char* format, va_list argptr)
{
    size_t needed = snprintf(NULL, 0, "[%s] %s\n", level, format) + 1;
    char* buf = malloc(needed);
    sprintf(buf, "[%s] %s\n", level, format);
    vfprintf(target, buf, argptr);
    free(buf);
}

void rawhttps_logger_level_set(enum Log_Level level)
{
    log_level = level;
}

void rawhttps_logger_log_debug(const char* format, ...)
{
    if (log_level > LOGGER_LOG_LEVEL_DEBUG) return;
    va_list argptr;
    va_start(argptr, format);
    logger_log_out(stdout, "DEBUG", format, argptr);
    va_end(argptr);
}

void rawhttps_logger_log_info(const char* format, ...)
{
    if (log_level > LOGGER_LOG_LEVEL_INFO) return;
    va_list argptr;
    va_start(argptr, format);
    logger_log_out(stdout, "INFO", format, argptr);
    va_end(argptr);
}

void rawhttps_logger_log_warning(const char* format, ...)
{
    if (log_level > LOGGER_LOG_LEVEL_WARNING) return;
    va_list argptr;
    va_start(argptr, format);
    logger_log_out(stderr, "WARNING", format, argptr);
    va_end(argptr);
}

void rawhttps_logger_log_error(const char* format, ...)
{
    if (log_level > LOGGER_LOG_LEVEL_ERROR) return;
    va_list argptr;
    va_start(argptr, format);
    logger_log_out(stderr, "ERROR", format, argptr);
    va_end(argptr);
}