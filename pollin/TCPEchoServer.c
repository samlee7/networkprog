#include "TCPEchoServer.h"  /* TCP echo server includes */
#include <sys/wait.h>       /* for waitpid() */
#include <sys/poll.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>

#define OPEN_MAX 5

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
	socklen_t			clilen;
	struct sockaddr_in  cliaddr;
	struct pollfd		client[OPEN_MAX]; 
	int i, maxi, nready, connfd, sockfd, recvMsgSize;
	char buf[32];
	int answers[5]={0};

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    servSock = CreateTCPServerSocket(echoServPort);

	
	client[0].fd=servSock;
	client[0].events = POLLIN;
	for(i=1; i<OPEN_MAX; i++) client[i].fd=-1;
	
	maxi=0;
	srand(time(NULL));
	int r = rand()%100+1;
	printf("%d\n",r);
	
    for (;;) 
    {
        nready=poll(client,maxi+1,3500);
		if(nready==-1){ perror("poll"); }
		else if(nready=0) { printf("Timeout \n"); }
		
		if(client[0].revents & POLLIN)
		{
			clilen=sizeof(cliaddr);
			connfd=accept(servSock, (struct sockaddr *) &cliaddr, &clilen);

			printf("handling new client \n");

			for(i=1; i<OPEN_MAX; i++)
				if(client[i].fd <0) {
					client[i].fd=connfd;
					break;
				}
			if(i==OPEN_MAX) perror("too many clients");
			client[i].events=POLLIN;
			if(i>maxi) maxi=i;
			if(--nready<=0) continue;
		}
		for(i=1; i<=maxi; i++) 
		{
			//printf("%d\n",maxi);
			if((sockfd=client[i].fd)<0) continue;
			if(client[i].revents & POLLIN){
				recvMsgSize = recv(sockfd, buf, 32, 0);
        			
				int ans=atoi(buf);
				answers[i]=ans;
				//send(sockfd, buf, 32, 0);
				
			}
		}

		if(answers[1]!=0 && answers[2]!=0 && answers[3]!=0 && answers[4]!=0)
		{
			int win=1;
			int closest;
			int n1=abs(r-answers[1]);	
			int n2=abs(r-answers[2]);
			int n3=abs(r-answers[3]);
			int n4=abs(r-answers[4]);
			closest=n1;
			if(n2<closest){ win=2; closest=n2; }
			if(n3<closest) { win=3; closest=n3; }
			if(n4<closest) { win=4; closest=n4; }
			for(i=1; i<maxi+1;i++)
			{
				client[i].events=POLLOUT;
				if(i==win)
				{
					char ee[]="you win";
					strcpy(buf,ee);
					if((sockfd=client[i].fd)<0) continue;
					if(client[i].revents & POLLOUT){
					send(sockfd, buf, 32, 0);
					}
				}
				else
				{
					char ee[]="you lose";
					strcpy(buf,ee);
					if((sockfd=client[i].fd)<0) continue;
					if(client[i].revents & POLLOUT){
					send(sockfd, buf, 32, 0);
					}
				}
			}	
	
		}

    }
    /* NOT REACHED */
}
