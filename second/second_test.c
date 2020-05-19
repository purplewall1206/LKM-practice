#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

int main() 
{
    int fd = 0;
    int counter = 0;
    int old_counter = 0;

    fd = open("/dev/second", O_RDONLY);
    printf("file open %d\n", fd);
    if (fd !=  - 1)
    {
        printf("loop\n");
        while (1) {
            printf("debug %d   %d\n", counter, old_counter);
            read(fd,&counter, sizeof(unsigned int));//读目前经历的秒数
            if(counter!=old_counter) {	
                printf("seconds after open /dev/second :%d\n",counter);
                old_counter = counter;
            }
            sleep(1);	
        }    
    }
    else
    {
        printf("Device open failure\n");
    }
    return 0;
}