#ifndef SYNFLOOD_H
#define SYNFLOOD_H
/*
    Syn Flood DOS with LINUX sockets
*/
#include "unp.h"
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/ip.h>    //Provides declarations for ip header
#include <pthread.h>
#include <syscall.h>
#define MYPORT 80
struct forSendFlood
{
    int socket;
    char * dgram;
    u_int16_t tlen;
    int flags;
    struct sockaddr * addr;
    size_t sizeAddr;
};
struct pseudo_header    //needed for checksum calculation
{
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;

    struct tcphdr tcp;
};
unsigned short csum(unsigned short *ptr,int nbytes);
void * func_2(void * argv);
void * synFlood(void * arg);
#endif // SYNFLOOD_H
