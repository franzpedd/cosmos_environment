#include "core/logger.h"

#ifdef COSMOS_LOGGER_ENABLED
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sstream>
#endif

#define LOG_MAX_SIZE 1024

namespace Cosmos
{
    const char* LogSeverity_cstr(LogSeverity severity) {
        switch (severity)
        {
            case LogSeverity::Trace: return "Trace";
            case LogSeverity::Todo: return "Todo";
            case LogSeverity::Info: return "Info";
            case LogSeverity::Warn: return "Warn";
            case LogSeverity::Assert: return "Assert";
        }
        
        return "Undefined";
    }

    void LogToTerminal(LogSeverity severity, const char* file, int line, const char* message, ...) {
        #ifdef COSMOS_LOGGER_ENABLED
        char buffer[LOG_MAX_SIZE];
        va_list args;
        va_start(args, message);
        vsnprintf(buffer, LOG_MAX_SIZE, message, args);
        va_end(args);

        time_t ttime = time(0);
        tm* local_time = localtime(&ttime);

        std::stringstream oss;
        oss << "[" << local_time->tm_mday << "/" << 1 + local_time->tm_mon << "/" << 1900 + local_time->tm_year;
        oss << " - " << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "]";
        oss << "[" << file << " - " << line << "]";
        oss << "[" << LogSeverity_cstr(severity) << "]";
        oss << ": " << buffer;

        printf("%s\n", oss.str().c_str());
        #endif
    }

    void LogToFile(LogSeverity severity, const char* path, const char* file, int line, const char* message, ...) {
        #ifdef COSMOS_LOGGER_ENABLED
        FILE* f = fopen(path, "a+");

        char buffer[LOG_MAX_SIZE];
        va_list args;
        va_start(args, message);
        vsnprintf(buffer, LOG_MAX_SIZE, message, args);
        va_end(args);

        time_t ttime = time(0);
        tm* local_time = localtime(&ttime);

        std::stringstream oss;
        oss << "[" << local_time->tm_mday << "/" << 1 + local_time->tm_mon << "/" << 1900 + local_time->tm_year;
        oss << " - " << local_time->tm_hour << ":" << local_time->tm_min << ":" << local_time->tm_sec << "]";
        oss << "[" << file << " - " << line << "]";
        oss << "[" << LogSeverity_cstr(severity) << "]";
        oss << ": " << buffer;

        fprintf(f, "%s\n", oss.str().c_str());
        fclose(f);
        #endif
    }
}
