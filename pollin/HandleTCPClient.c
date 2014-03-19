#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */
	char check[RCVBUFSIZE]="";
	char ee[]="EXIT";
	strcpy(check,ee);

	memset(echoBuffer,0,32);
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");

	if(strcmp(echoBuffer,check)==0)
	{
		printf("EXIT Command Received\n");
		close(clntSocket);
		recvMsgSize=0;
	}
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
    {
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
		if(strcmp(echoBuffer,check)==0)
		{
			printf("EXIT Command Received\n");
			close(clntSocket);
			recvMsgSize=0;
		}
    }

    close(clntSocket);    /* Close client socket */
}
