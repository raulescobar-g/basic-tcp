#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int backlog = 5;

int main() {
  struct addrinfo *result, *addr_iterator, hints;
  int socket_fd;
  // config
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addrinfo = getaddrinfo(NULL, "3000", &hints, &result);
  if (addrinfo != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo));
    exit(1);
  }

  for (addr_iterator = result; addr_iterator != NULL;
       addr_iterator = addr_iterator->ai_next) {
    socket_fd = socket(addr_iterator->ai_family, addr_iterator->ai_socktype,
                       addr_iterator->ai_protocol);
    if (socket_fd == -1) {
      perror("socket()\n");
      continue;
    }

    if (bind(socket_fd, addr_iterator->ai_addr, addr_iterator->ai_addrlen) == 0)
      break;

    close(socket_fd);
  }

  freeaddrinfo(result);

  if (addr_iterator == NULL) {
    perror("Could not bind\n");
    exit(1);
  }

  int listen_code = listen(socket_fd, backlog);
  if (listen_code == -1) {
    perror("listen()\n");
    exit(1);
  }

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  int accept_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);

  if (accept_fd == -1) {
    perror("accept()\n");
    exit(1);
  }
  char *msg = "howdy";
  int bytes_sent = send(accept_fd, msg, strlen(msg), 0);

  close(accept_fd);
  close(socket_fd);

  return 0;
}
