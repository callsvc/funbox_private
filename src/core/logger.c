

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <types.h>
#include <logger.h>
logger_t *logs = nullptr;

void logger_init() {
    logs = fb_malloc(sizeof(logger_t));
    pthread_mutex_init(&logs->mutex, nullptr);

    logs->threshold = sizeof(logs->logm) / 100;
    *logs->logm = '?';
}
void logger_flush(logger_t *logger, bool lock);
void logger_destroy() {

    logger_flush(logs, true);
    pthread_mutex_destroy(&logs->mutex);
    fb_free(logs);
    logs = nullptr;
}

void logger_flush(logger_t *logger, const bool lock) {
    if (lock)
        pthread_mutex_lock(&logger->mutex);

    const char *logm = logger->logm;
    const size_t lines = logger->count;
    for (size_t i = 0; i < lines; i++) {
        char message[1024];
        snprintf(message, strchr(logm, '\n') - logm + 2, "%s", logger->logm);
        if (!logger->count--)
            break;
        if (!logger->file)
            fputs(message, stderr);

        logm = strchr(logm, '\n');
        if (!logm || *logm == '?')
            break;
        logm++;
    }
    memset(logger->logm, '\0', sizeof(logger->logm));
    *logger->logm = '?';
    logger->count = 0;

    if (lock)
        pthread_mutex_unlock(&logger->mutex);
}

void logger_write(logger_t *logger, const logm_type_e type, const char * file, const uint64_t line, const char *fmt, ...) {
    va_list args = {};
    va_start(args, fmt);

    pthread_mutex_lock(&logs->mutex);

    char log[150];
    vsnprintf(log, 1024, fmt, args);
    const char * type_str = nullptr;

    switch(type) {
        case logm_type_success: type_str = "success";   break;
        case logm_type_error:   type_str = "error";     break;
        case logm_type_warning: type_str = "warning";   break;
        case logm_type_info:    type_str = "info";      break;
        case logm_type_debug:   type_str = "debug";     break;
        case logm_type_trace:   type_str = "trace";     break;
        default:
    }

    sprintf(strchr(logger->logm, '?'), "(%s), %s:%lu = %s\n?", type_str, strrchr(file, '/') + 1, line, log);
    logger->count++;

    if (logger->count == logger->threshold)
        logger_flush(logger, false);

    pthread_mutex_unlock(&logs->mutex);

    va_end(args);
}
