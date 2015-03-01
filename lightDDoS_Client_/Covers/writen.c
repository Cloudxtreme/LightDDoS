/* include writen */
#include	"unp.h"

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

void Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
		err_sys("writen error");
}
void sendString(FILE * fp, int sockfd)
{
    printf("Password:\n>");
    char	sendline[MAXLINE];
    if(Fgets(sendline, MAXLINE, fp) != NULL)
        Writen(sockfd, sendline, strlen(sendline));
}

void majorClient(int * k, int * s , char * buffer)
{

    if(*k)
    {
        printf("Enter please \"stop\" for exit\n>");
        scanf("%s",buffer);
        while (strcmp(buffer,"stop"))
        {
            printf("Incorrect string, repeat please\n>");
            scanf("%s",buffer);
        }
        Writen(*s, buffer, strlen(buffer));
        *k = 0;
    }
}
