#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
int main()
{
    char str[1024],str2[1024];
    int fd,stop;
    int ret;
    fd=open("/dev/gatieme",O_RDWR,S_IRUSR|S_IWUSR);
    printf("Dev number %d\n",fd);
    if(fd != -1)
    {
        printf("Please input the string written to globalmem:\n");
        scanf("%s",str);
        printf("The str is %s\n",str);
        ioctl(fd,1,NULL);
        ret=write(fd,str,strlen(str));
        printf("The ret is %d\n",ret);
        lseek(fd,0,SEEK_SET);
        scanf("%d",&stop);
        read(fd,str2,100);
        printf(" The globalmem is %s\n",str2);
        close(fd);
    }else{
        printf("Device open failure\n");
    }
}
