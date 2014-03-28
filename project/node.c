/*  Alex Barke 			    */
/*  Sam Lee 		        */
/*                          */
/*  CS 164 Project          */
/*  node.c                  */


#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/poll.h>
#include <sys/wait.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>

/******************************************************************************
hostname_to_ip function:
	This function takes in hostname array and ip array and converts it to 
	ip address.	
	Example of usage: hostname=alpha-45.cs.ucr.edu ip=char* of size 100
			  returns ip populated with 169.235.29.150

******************************************************************************/
 int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
         
    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 1;
    }
 
    addr_list = (struct in_addr **) he->h_addr_list;
     
    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
     
    return 1;
}
/******************************************************************************
struct frame:
	Holds the structure of the frame.
	Message Type - DEST Address - SRC Address - Data
	Represented by:
	msgType	       destAddr       srcAddr       data

******************************************************************************/ 
struct Frame
{
	char msgType[2];
	unsigned char destAddr[2];
	unsigned char srcAddr[2];
	char data[1024];
} Frame;

/******************************************************************************
createFrame:
	This function populates the frame that will be sent to the repeater.
	The frame will be populated with msgtype, both addresses and data

******************************************************************************/ 
struct Frame* createFrame(char* msgType, char* destAddr, char* srcAddr, 
								char* data)
{
	struct Frame* newFrame = (struct Frame*)malloc(sizeof(struct Frame));
	strcpy(newFrame->msgType,msgType);
	
    //tokenize the address
	unsigned char* token = strtok(destAddr,".");
	token = strtok(NULL,".");
	unsigned char* token2 = strtok(NULL,".");
	unsigned char* token3 = strtok(NULL,".");
	
	int da1 = atoi(token2);
	int da2 = atoi(token3);
	unsigned char destAddr1 = da1;
	unsigned char destAddr2 = da2;
	unsigned char newDestAddr[2];
	newDestAddr[0] = destAddr1;
	newDestAddr[1] = destAddr2; 
	
	token = strtok(srcAddr,".");
	token = strtok(NULL,".");
	token2 = strtok(NULL,".");
	token3 = strtok(NULL,".");
	
	int sa1 = atoi(token2);
	int sa2 = atoi(token3);
	unsigned char srcAddr1 = sa1;
	unsigned char srcAddr2 = sa2;
	unsigned char newSrcAddr[2];
	newSrcAddr[0] = srcAddr1;
	newSrcAddr[1] = srcAddr2;
	
    //store the data into newFrame
	strcpy(newFrame->destAddr,newDestAddr);
	strcpy(newFrame->srcAddr,newSrcAddr);
	strcpy(newFrame->data,data);
	
	return newFrame;
	
} 

/******************************************************************************
printFrame:
	This functions is mainly used for testing purposes. This prints out the
	elements of the frame to test correctness.

******************************************************************************/ 
void printFrame(struct Frame* printFrame)
{
	printf("\n---Printing Frame---\n");
	printf("Message Type: %s\n",printFrame->msgType);
	printf("Dest Addr: %s\n",printFrame->destAddr);
	printf("Data: %s\n",printFrame->data);
}
 

/******************************************************************************
sendFrame:
	This function is used to send a whole frame to the repeater

******************************************************************************/  
int sendFrame(int sock,struct Frame* sendFrame)
{	
	if( send(sock , sendFrame->msgType , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame->destAddr , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame->srcAddr , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame->data , 1024 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        return 0;
} 
 
 
 
 
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
    
    if(argc != 4)
    {
    	printf("Incorrect Command Line Args!\nPlease use format: <node> ");
    	printf("<NodeIPAddr> <RepIPAddr> <Port>\n");
    	exit(1);
    	
    }
    printf("Node:\n\tConnecting to IPAddr: %s\n\tPort: %s\n",argv[2],argv[3]);
    char* getIPAddr;
    char* getIPAddr2;
    struct hostent* host = gethostbyname(argv[1]);
    char** current_addr = host->h_addr_list;
    struct in_addr* addr = (struct in_addr*)(*current_addr); 
    getIPAddr = inet_ntoa(*addr); 
    
    /* Convert user input to ip address */   
    char *hostname = argv[1];
    char ip[100];
     
    hostname_to_ip(hostname , ip);
    printf("%s resolved to %s" , hostname , ip);
     
    printf("\n");
    
    
    char *hostname2 = argv[2];
    char ip2[100];
     
    hostname_to_ip(hostname2 , ip2);
    printf("%s resolved to %s" , hostname2 , ip2);
     
    printf("\n");
    
    /* Format hostent info for repeater */
    struct hostent* host2 = gethostbyname(argv[2]);
    char** current_addr2 = host2->h_addr_list;
    struct in_addr* addr2 = (struct in_addr*)(*current_addr2); 
    getIPAddr2 = inet_ntoa(*addr2); 

    /*Create socket to connect to repeater */
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    server.sin_addr.s_addr = inet_addr(argv[2]);
    server.sin_family = AF_INET;
    server.sin_port=htons(atoi(argv[3]));
    server.sin_addr.s_addr=*(long*)host2->h_addr_list[0]; /* set the addr */
 	
    /*Make connection to server */
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
    //keep communicating with server
    char messageType[2];
    struct Frame* myFrame = NULL;
    char message_data[1024];
    char data_dest[1024];
    char msgType[2];
    char trainingMsg2[2];
    char trainingMsg3[2];
    char trainingMsg4[1024];
    int recvMsgSize;
    char* base = malloc(sizeof(char)*100);
    int msgType_int;
    int atoi_msg_type;
    char ip_t[100];
    char ip_t2[100];
    while(1)
    {
        //user options
    	printf("Enter message type: \n");
    	printf("\t1 for request normal\n");
    	printf("\t2 for request high\n");
    	printf("\t4 for data packet\n");
    	printf("\t11 for training message\n");
        scanf("%s" , messageType);
        atoi_msg_type = atoi(messageType);
        int four = 4;
        
        strcpy(ip_t,ip);
        strcpy(ip_t2,ip2);
        if(  atoi_msg_type == 4 )
        {
        	printf("Enter hostname of who you want to send to: \n");
        	scanf("%s",data_dest);
        	hostname_to_ip(data_dest,ip_t);
        	printf("Enter message data: \n");
        	scanf("%s",message_data);
        	myFrame = createFrame(messageType,ip_t,ip_t2,message_data);

        }
        else
        {
        	myFrame = createFrame(messageType,ip_t,ip_t2,"");
        }

    	if(sendFrame(sock,myFrame) == 0)
    		printf("send successful\n");
    	
         
      		
		if((recvMsgSize=recv(sock,msgType,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		if((recvMsgSize=recv(sock,trainingMsg2,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
        
		int a = trainingMsg2[0];
		int b = trainingMsg2[1];
		
		
		
		if((recvMsgSize=recv(sock,trainingMsg3,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		int c = trainingMsg3[0];
		int d = trainingMsg3[1];
		
		if((recvMsgSize=recv(sock,trainingMsg4,1024,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}	
		
		sprintf(base,"169.235.%d.%d",c,d);
		printf("%s\n\n",base);
		
		msgType_int = atoi(msgType);
		
		char hostname[1024];

		char ip[100];

		unsigned char* token;

		unsigned char* token2;
		unsigned char* token3;
		
		int dadd1;
		int dadd2;
		
		switch(msgType_int)
		{
			case 0: printf("You received an Idle Down Frame\n");
			break;
			case 3: printf("You received a grant Frame\n");
			break;
			case 4: printf("Data received from repeater\n");

				gethostname(hostname,1014);
				hostname_to_ip(hostname,ip);
				token = strtok(ip,".");
				token = strtok(NULL,".");
				token2 = strtok(NULL,".");
				token3 = strtok(NULL,".");
				printf("token2: %s\n",token2);
				dadd1 = atoi(token2);
				dadd2 = atoi(token3);
				printf("da1 is: %d\nda2 is: %d\n",da1,da2);
				if(dadd1 == a && dadd2 == b)
				{
					printf("You received data: "); 
					printf("%s\n",trainingMsg4);
				}
				
			break;
			case 5: printf("Received an incoming Frame\n");
			break;
			case 6: printf("Received an idle up Frame\n");
			break;
			case 11: printf("Received a training message Frame\n");
			break;
			default: printf("I don't know what was sent\n");
			break;
		}
		
		memset(messageType,0,sizeof messageType);
		myFrame = NULL;
		memset(message_data,0,sizeof message_data);
		memset(msgType,0,sizeof msgType);
		memset(trainingMsg2,0,sizeof trainingMsg2);
		memset(trainingMsg3,0,sizeof trainingMsg3);
		memset(trainingMsg4,0,sizeof trainingMsg4);
		memset(base,0,sizeof base);

    }
     
    close(sock);
    return 0;
}
