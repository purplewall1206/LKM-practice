#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main()
{
    char sendbuf[102400], recvbuf[1024];
    int fd = 0, stop = 0;
    int ret = 0;
    fd = open("/dev/gatieme", O_RDWR, S_IRUSR|S_IWUSR);
    printf("Dev number %d AKA %x\n", fd, fd);
    int n = 0;
    // while (1) {
    //     printf("0->exit   1->read   2->write   3->clear   4->setpos\n");
    //     scanf("%d", &n);
    //     switch(n) {
    //         case 0:
    //             close(fd);
    //             return 0;
    //         case 1:
    //             read(fd, recvbuf, 1024000);
    //             printf("read : %s\n", recvbuf);
    //             break;
    //         case 2:
    //             scanf("%s", sendbuf);
    //             ret = write(fd, sendbuf, strlen(sendbuf));
    //             printf("The ret is %d\n",ret);
    //             break;
    //         case 3:
    //             ioctl(fd, 1, NULL);
    //             break;
    //         case 4:
    //             lseek(fd, 0, SEEK_SET);
    //             break;
    //         default:
    //             break;
    //     }

    //     printf("loop\n");
    // }
    strcpy(sendbuf, "aaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbccccccccccc");
    ret = write(fd, sendbuf, strlen(sendbuf));
    printf("The ret is %d\n",ret);
    lseek(fd, 0, SEEK_SET);
    read(fd, recvbuf, 102400);
    printf("read : %s\n", recvbuf);
    ioctl(fd, 1, NULL);
    close(fd);
    return 0;
}