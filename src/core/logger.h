#pragma once
#include <pthread.h>
#include <stddef.h>

typedef struct fsfile fsfile_t;

typedef enum logm_type {
    logm_type_success,
    logm_type_error,
    logm_type_warning,
    logm_type_info,
    logm_type_debug,
    logm_type_trace,
} logm_type_e;

typedef struct logger {
    char logm[3 * 1024];
    char scratch[1024];

    size_t count;
    size_t threshold;

    fsfile_t *file;
    size_t filepos;
    pthread_mutex_t mutex;
} logger_t;

extern logger_t *logs;

void logger_init();
void logger_destroy();

void logger_write(logger_t *, logm_type_e, const char *, uint64_t line, const char *, ...);
#define logger_success(fmt, ...)\
    logger_write(logs, logm_type_success, __FILE__, __LINE__, fmt, __VA_ARGS__)

#define logger_info(fmt, ...)\
    logger_write(logs, logm_type_info, __FILE__, __LINE__, fmt, __VA_ARGS__)

