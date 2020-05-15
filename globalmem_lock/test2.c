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
    
    strcpy(sendbuf, "1111111111111111111122aaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbccccccccccc");

    struct timeval start, end;
    long long writetime[10000], readtime[10000], ioctltime[10000];
    long long res[3];
    for (int i = 0;i < 10;i++) {
        gettimeofday(&start, NULL);
        ret = write(fd, sendbuf, strlen(sendbuf));
        gettimeofday(&end, NULL);
        writetime[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

        lseek(fd, 0, SEEK_SET);

        gettimeofday(&start, NULL);
        read(fd, recvbuf, 102400);
        gettimeofday(&end, NULL);
        readtime[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);


        gettimeofday(&start, NULL);
        // MEM_CLEAR
        ioctl(fd, 1, NULL);
        gettimeofday(&end, NULL);
        ioctltime[i] = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
        if (i % 100 == 0) {
            printf("%d has finished\n", i+1);
        }
        res[0] += writetime[i];
        res[1] += readtime[i];
        res[2] += ioctltime[i];
    }
    
    printf("write : %lld us   \nread  : %lld us   \nioctl : %lld us   \n", res[0], res[1], res[2]);
    close(fd);
    return 0;
}