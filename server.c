#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http.c"
#include "logger.c"

int backlog = 5;

int main(int argc, char *argv[]) {
  int opt;
  char *port = "3000";

  while ((opt = getopt(argc, argv, "v::p::")) != -1) {
    switch (opt) {
    case 'p':
      port = optarg;
      break;
    case 'v':
      if (!strcmp(optarg, "trace"))
        r_log_level = r_trace;
      if (!strcmp(optarg, "debug"))
        r_log_level = r_debug;
      if (!strcmp(optarg, "info"))
        r_log_level = r_info;
      if (!strcmp(optarg, "warn"))
        r_log_level = r_warn;
      if (!strcmp(optarg, "error"))
        r_log_level = r_error;
      break;
    default:
      fprintf(stderr, "Usage: %s [-p port] [-v level]\n", argv[0]);
      exit(1);
    }
  }

  r_trace("initializing variables, and setting addrinfo hints");
  struct addrinfo *result, *addr_iterator, hints;
  int socket_fd;
  // config
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addrinfo = getaddrinfo(NULL, port, &hints, &result);
  if (addrinfo != 0) {
    r_error("Error getting addrinfo %s\n", gai_strerror(addrinfo));
    exit(1);
  }

  for (addr_iterator = result; addr_iterator != NULL;
       addr_iterator = addr_iterator->ai_next) {
    r_trace("iterating over addr list");
    socket_fd = socket(addr_iterator->ai_family, addr_iterator->ai_socktype,
                       addr_iterator->ai_protocol);
    if (socket_fd == -1) {
      r_error("socket()\n");
      continue;
    }

    if (bind(socket_fd, addr_iterator->ai_addr, addr_iterator->ai_addrlen) ==
        0) {
      r_trace("successful bind()");
      break;
    }

    r_trace("bind unsuccessful, closing socket");
    close(socket_fd);
  }

  r_trace("freeing addrinfo");
  freeaddrinfo(result);

  if (addr_iterator == NULL) {
    r_error("Could not bind to any addrinfo \n");
    exit(1);
  }

  r_trace("attempting listen()");
  int listen_code = listen(socket_fd, backlog);
  if (listen_code == -1) {
    r_error("listen()\n");
    exit(1);
  }

  r_info("Listening on port: %s", port);

  while (1) {

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    r_trace("accepting connection");
    int accept_fd =
        accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);

    if (accept_fd == -1) {
      r_error("accept()\n");
      exit(1);
    }

    char buf[1024];
    int bytes_recv = recv(accept_fd, buf, 1024, 0);
    if (bytes_recv == -1) {
      r_error("recv()\n");
      exit(1);
    }

    http_request req = {0};
    int valid = parse_request(buf, &req);

    char *response;

    if (valid < 0) {
      r_error("%d", valid);
      switch (valid) {
      case -1:
        break;
      case -2:
        break;
      default:
      }
      response = "HTTP/1.1 400 Bad request\r\n"
                 "Content-type: text/plain; charset=utf-8\r\n"
                 "\r\n"
                 "Bad requesta";
    } else {
      response = "HTTP/1.1 200 OK\r\n"
                 "Content-type: text/plain; charset=utf-8\r\n"
                 "\r\n"
                 "howdy";
    }

    free_request(&req);

    r_trace("sending msg");
    int bytes_sent = send(accept_fd, response, strlen(response), 0);
    if (bytes_sent == -1) {
      r_error("send()\n");
      exit(1);
    }
    close(accept_fd);
  }
  close(socket_fd);

  return 0;
}
