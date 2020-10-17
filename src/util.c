#include "common.h"
#include "util.h"

char *get_datetime(void) {
  time_t current_time;
  char *c_time_string;

  current_time = time(NULL);
  c_time_string = ctime(&current_time);

  return strtok(c_time_string, "\n");
}

// Parse and validate input
char *parse_input(int input_num, char *input_str[])
{
  char *suffix, *resolved_path;
  char *uri = NULL;
  GError *error;

  if (input_num != 2) {
    printf("readerx: missing file operand\
        \nUsage: readerx FILE\n");
  }
  else {
    resolved_path = realpath(input_str[1], NULL);

    if (!resolved_path)
      printf("readerx: File does not exist\n");
    else {
      error = NULL;
      uri = g_filename_to_uri(resolved_path, NULL, &error);
    }
    free(resolved_path);
  }

  return uri;
}

int enqueue(queue_t *queue, void *item)
{
  if ((queue->head == 0 && queue->tail == (MAX_QUEUE_LENGTH - 1))
      || ((queue->head - queue->tail) == 1)) {
    LOG("Queue overflow occured");
    return -1;
  }

  queue->q[queue->tail] = item;
  if (queue->tail == MAX_QUEUE_LENGTH - 1)
    queue->tail = 0;
  else
    queue->tail++;

  LOG("head: %d, tail: %d", queue->head, queue->tail);
  return 0; 
}

void *dequeue(queue_t *queue)
{
  void *item;

  if (queue->head == queue->tail) {
    LOG("Queue underflow occured");
    return NULL;
  }

  item = queue->q[queue->head];
  if (queue->head == MAX_QUEUE_LENGTH - 1)
    queue->head = 0;
  else
    queue->head++;

  LOG("head: %d, tail: %d", queue->head, queue->tail);
  return item;
}

int readerx_log(char *file, const char *func, int line, char *fmt, ...)
{
  static FILE *logger;
  va_list ap;
  char log_message[STR_MAX];

  logger = fopen(LOG_FILEPATH, "a");

  sprintf(log_message, "%s at %s (%s:%d) :", get_datetime(), func, file, line);
  strcat(log_message, fmt);
  strcat(log_message, "\n");

  va_start(ap, fmt);
  vfprintf(logger, log_message, ap);
  va_end(ap);

  fclose(logger);
  return 0;
}
