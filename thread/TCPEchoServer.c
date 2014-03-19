#include "TCPEchoServer.h"  /* TCP echo server includes */
#include <sys/wait.h>       /* for waitpid() */
#include <sys/poll.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define OPEN_MAX 5
 
int answers[5]={0};  
int cnum=1;
int numsol=0;
int r;

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

void * process(void * ptr)
{
	char buffer[32];
	int len=32;
    int i=cnum;
    int ans;
	printf("cnum is %d\n", cnum);
	connection_t * conn;
	long addr = 0;
	cnum++;

	if (!ptr) pthread_exit(0); 
	conn = (connection_t *)ptr;

    while(1)
	{
	    if(recv(conn->sock, buffer, 32,0)>0)
        {
	        ans=atoi(buffer);
	        answers[numsol]=ans; 
            numsol++;
			printf("numsol is %d\n",numsol);
			//sleep(5);
			break;
        } 
        //if(numsol==3) break;  
    }
	//printf("Calculating winner\n");
	while(1){
    if(numsol==3)
	{
			int win=1;
			int closest;
			int n1=abs(r-answers[0]);	
			int n2=abs(r-answers[1]);
            int n3=abs(r-answers[2]);
			//printf("%d %d %d\n",n1,n2,n3);
			closest=n1;
			if(n2<closest){ win=2; closest=n2; }
            if(n3<closest){ win=3; closest=n3; }
            if(i==win)
			{
				char ee[]="you win";
				strcpy(buffer,ee);
				send(conn->sock, buffer, 32, 0);
					
			}
			else
			{
				char ee[]="you lose";
				strcpy(buffer,ee);
				send(conn->sock, buffer, 32, 0);
					
			}
			printf("exiting thread\n");
	        close(conn->sock);
	        free(conn);
            pthread_exit(0);
    
    }
	}


}


int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
	socklen_t			clilen;
	struct sockaddr_in  cliaddr;
    connection_t*       connection;
    pthread_t           thread;
	int  connfd, sockfd, recvMsgSize;
	char buf[32];
    int socks[5];
    int i=0;

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    servSock = CreateTCPServerSocket(echoServPort);	
	srand(time(0));
    r = rand()%100+1;
    printf("%d\n",r);

    while(1)
    {
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(servSock, &connection->address, &connection->addr_len);
        if(connection->sock<=0) { 
            free(connection); 
        }
        else 
        {
		    pthread_create(&thread,0,process, (void *)connection);
			pthread_detach(thread);
            socks[i]=connection->sock;
            i++;
        }
		if(numsol==3) break;
	}
			
		

    /* NOT REACHED */
}
