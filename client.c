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
  int max_buff = 1024;
  char buf[max_buff];

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

    if (connect(socket_fd, addr_iterator->ai_addr, addr_iterator->ai_addrlen) ==
        0)
      break;

    close(socket_fd);
  }

  freeaddrinfo(result);

  if (addr_iterator == NULL) {
    perror("Could not bind\n");
    exit(1);
  }

  int numbytes;
  if ((numbytes = recv(socket_fd, buf, max_buff - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  printf("%s\n", buf);

  close(socket_fd);
  return 0;
}
