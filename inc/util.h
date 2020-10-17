#ifndef UTIL_H
#define UTIL_H

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <glib-2.0/glib.h>

//#define LOG_ENABLE

#define STR_MAX 400
#define LOG_FILEPATH "./readerx_log.txt"

#ifdef LOG_ENABLE
#define LOG(...) \
  readerx_log(__FILE__, __func__, __LINE__, __VA_ARGS__);
#else
#define LOG(...) do {} while(0);
#endif

char *get_datetime(void);
char *parse_input(int input_num, char **input_str);
int enqueue(queue_t *queue, void *item);
void *dequeue(queue_t *queue);
int readerx_log(char *file, const char *func, int line, char *fmt, ...);
#endif
