#include "logger.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_HEADERS = 256;

typedef enum {
  UNIMPLEMENTED_METHOD = -1,
  INVALID_PATH = -2,
  INVALID_HTTP_VERSION = -3,
  INVALID_HEADER = -4,
  TOO_MANY_HEADERS = -5,
} http_parse_errors;

typedef struct {
  char *method;
  char *path;
  char *http_version;
  char *body;
  int headerlen;
  char *headers[MAX_HEADERS];
} http_request;

void free_request(http_request *raw_req) {
  free(raw_req->http_version);
  for (int i = 0; i < raw_req->headerlen; ++i) {
    free(raw_req->headers[i]);
  }
  free(raw_req->path);
  free(raw_req->method);
  free(raw_req->body);
}

int validate_method(char *method) {
  r_error("%s", method);
  if (!strcmp(method, "GET")) {
    return 0;
  }
  if (!strcmp(method, "POST")) {
    return 0;
  }
  if (!strcmp(method, "PUT")) {
    return 0;
  }
  if (!strcmp(method, "DELETE")) {
    return 0;
  }
  if (!strcmp(method, "OPTIONS")) {
    return 0;
  }
  if (!strcmp(method, "HEAD")) {
    return 0;
  }
  return UNIMPLEMENTED_METHOD;
}

int validate_path(char *path, char *method) {
  // TODO: absolute form

  // asterisk-form
  if (!strcmp(method, "OPTIONS") && !strcmp(path, "*")) {
    return 0;
  }
  // origin-form
  if (*path == '/') {
    // TODO: validate path
    return 0;
  }
  return INVALID_PATH;
}

int validate_version(char *version) {
  if (!strcmp(version, "HTTP/1.1")) {
    return 0;
  }
  return INVALID_HTTP_VERSION;
}

int parse_request(char *raw_req, http_request *parsed_req) {
  char *buf;
  int len;

  char *start = raw_req;
  char *end = raw_req;
  for (end = raw_req; *end != ' '; ++end)
    ;

  len = end - start;
  buf = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(buf, start, len);
  buf[len] = '\0';
  int valid_method = validate_method(buf);
  if (valid_method < 0) {
    return valid_method;
  }

  parsed_req->method = buf;
  r_trace("[%s]", parsed_req->method);

  start = end + 1;
  for (end = start; *end != ' '; ++end)
    ;
  len = end - start;
  buf = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(buf, start, len);
  buf[len] = '\0';

  int valid_path = validate_path(buf, parsed_req->method);
  if (valid_path < 0) {
    return valid_path;
  }

  parsed_req->path = buf;
  r_trace("[%s]", parsed_req->path);

  start = end + 1;
  end = start + 1;
  for (; *end != '\n' && *(end - 1) != '\r'; ++end)
    ;

  len = end - start - 1;
  buf = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(buf, start, len);
  buf[len] = '\0';

  int valid_version = validate_version(buf);
  if (valid_version < 0) {
    return valid_version;
  }
  parsed_req->http_version = buf;
  r_trace("[%s]", parsed_req->http_version);

  start = end + 1;
  end = start + 1;
  for (; *end != '\n' && *(end - 1) != '\r'; ++end)
    ;

  len = end - start - 1;
  buf = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(buf, start, len);
  buf[len] = '\0';

  while (strcmp(buf, "") != 0) {
    r_trace("%s", buf);
    parsed_req->headers[parsed_req->headerlen] = buf;
    parsed_req->headerlen += 1;

    start = end + 1;
    end = start + 1;
    for (; *end != '\n' && *(end - 1) != '\r'; ++end)
      ;
    len = end - start - 1;
    buf = (char *)malloc(sizeof(char) * (len + 1));
    strncpy(buf, start, len);
    buf[len] = '\0';
  }

  start = end + 1;
  end = start;
  for (; *end != '\0'; ++end)
    ;
  len = end - start;
  buf = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(buf, start, len);
  buf[len] = '\0';

  parsed_req->body = buf;
  r_trace("[%s]", parsed_req->body);

  return 0;
}
