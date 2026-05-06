#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
  int fd = open("/dev/events", O_RDONLY);
  volatile int j = 0;
  while(1){
    j ++;
    if (j == 1000000) {
      char buf[256];
      int n = read(fd, buf, 255);
      if (n > 0) {
        buf[n] = '\0';
        printf("receive event: %s", buf);
      } else {
        printf("read error or EOF, n=%d\n", n);
      }
      j = 0;
    }
  }
  return 0;
}
