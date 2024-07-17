#ifndef LOGGER
#define LOGGER

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

enum verbosity {
  r_trace = 0,
  r_debug = 1,
  r_info = 2,
  r_warn = 3,
  r_error = 4
} verbosity;

static enum verbosity r_log_level = 0;

void _r_log(int level, FILE *file, const char *fmt, ...) {
  if (r_log_level > level) {
    return;
  }

  va_list ap;
  time_t t;
  char datestr[51];

  char *level_str = "TRACE";
  if (level == r_debug)
    level_str = "DEBUG";
  if (level == r_info)
    level_str = "INFO ";
  if (level == r_warn)
    level_str = "WARN ";
  if (level == r_error)
    level_str = "ERROR";

  /* determine if we just go to std error */
  file = (file == NULL) ? stderr : file;

  /* datetime & pid formatting */
  t = time(NULL);
  tzset();
  strftime(datestr, sizeof(datestr) - 1, "%a %b %d %T %Z %Y", localtime(&t));
  fprintf(file, "[%s] %s (%d): ", level_str, datestr, getpid());

  /* draw out the vararg format */
  va_start(ap, fmt);
  vfprintf(file, fmt, ap);
  va_end(ap);

  /* bump to the next line */
  fprintf(file, "\n");
}

#define r_trace(...) _r_log(0, NULL, __VA_ARGS__)
#define r_debug(...) _r_log(1, NULL, __VA_ARGS__)
#define r_info(...) _r_log(2, NULL, __VA_ARGS__)
#define r_warn(...) _r_log(3, NULL, __VA_ARGS__)
#define r_error(...) _r_log(4, NULL, __VA_ARGS__)

#endif
