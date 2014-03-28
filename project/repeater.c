/*  Alex Barke 			    */
/*  Sam Lee 		        */
/*                          */
/*  CS 164 Project          */
/*  repeater.c              */


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


int allAddrs[50];
int addrscount = 0;
int m_num=0;
char ip[100];
int argc_global;

/******************************************************************************
struct Request:
	Holds the structure of the request.
	Request stores the ip address of the source and the priority

******************************************************************************/ 
struct Request
{
	char* ip;
	int request_priority;
	int sock;
} Request;

struct Request request_arr[100];
int request_index = 0;



/******************************************************************************
struct Frame:
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
sendFrame:
	This function is used to send a whole frame to the repeater

******************************************************************************/  
int sendFrame(int sock,struct Frame* sendFrame1)
{
	//char* buffer = malloc(sizeof(char)* 1030);
	//strcpy(buffer,sendFrame->msgType);
	//strcat(buffer,sendFrame->destAddr);
	//strcat(buffer,sendFrame->srcAddr);
	//strcat(buffer,sendFrame->data);
	
	if( send(sock , sendFrame1->msgType , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame1->destAddr , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame1->srcAddr , 2 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        if( send(sock , sendFrame1->data , 1024 , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        return 0;
}
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
	
	if(destAddr != "")
	{
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
	    strcpy(newFrame->destAddr,newDestAddr);
	}
	else
	{
		strcpy(newFrame->destAddr,"");
	}
	if(srcAddr != "")
	{
	
	    unsigned char*token = strtok(srcAddr,".");
	    token = strtok(NULL,".");
	    unsigned char*token2 = strtok(NULL,".");
	    unsigned char*token3 = strtok(NULL,".");
	
	    int sa1 = atoi(token2);
	    int sa2 = atoi(token3);
	    unsigned char srcAddr1 = sa1;
	    unsigned char srcAddr2 = sa2;
	    unsigned char newSrcAddr[2];
	    newSrcAddr[0] = srcAddr1;
	    newSrcAddr[1] = srcAddr2;
	    strcpy(newFrame->srcAddr,newSrcAddr);
	}
	else
		strcpy(newFrame->srcAddr,"");
	
	strcpy(newFrame->data,data);
	
	
	return newFrame;
	
} 


void *connection_handler(void *);
/******************************************************************************
main:
	Calculates the ip address, makes connection, and creates the thread

******************************************************************************/ 
int main(int argc, char* argv[])
{
	if(argc < 3 ||  argc > 4)
	{
		printf("Incorrect command line args <Src> <port>  <[upstreamIP:upstream port]>\n");
		exit(1);	
	}
	char* hostname = argv[1];
	int port = atoi(argv[2]);
	char* addrport = argv[3];
	char* pch;
	pch = strtok(addrport,":");
	char* upIP = pch;
	pch = strtok(NULL,":");
	char* upPORT = pch;
	argc_global=argc;
	
	char* getIPAddr;
	struct hostent* host = gethostbyname(argv[1]);
	char** current_addr = host->h_addr_list;
	struct in_addr* addr = (struct in_addr*)(*current_addr); 
    //printf("%s\n",inet_ntoa(*addr));
    getIPAddr = inet_ntoa(*addr); 
    
    /* Convert user input to ip address */   
    hostname_to_ip(hostname , ip);
		
	printf("addr: %s \nport: %s\n\n", upIP,upPORT);
	
	if(argc == 4)
	{
		struct sockaddr_in eServAdder;
		int sock;
		if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
		{
			perror("socket() failed");
			exit(1);
		}
		
		char* ip2 = malloc(sizeof(char)*100);
		hostname_to_ip(upIP,ip2);
		
		
		memset(&eServAdder,0,sizeof(eServAdder));
		eServAdder.sin_family=AF_INET;
		eServAdder.sin_port=htons(atoi(upPORT));
		eServAdder.sin_addr.s_addr=inet_addr(ip2);	
			
		if(connect(sock,(struct sockaddr*)&eServAdder,sizeof(eServAdder))<0)
		{
			perror("connect() failed");
			exit(1);
		}
		
		printf("Sending training message to parent repeater\n");
		char* destIPcopy = malloc(sizeof(char)*200);
		strcpy(destIPcopy,upIP);
		//char* ipCopy = malloc(sizeof(char) * 100);
		//strcpy(ipCopy,ip);
		struct Frame* tFrame = createFrame("11",destIPcopy,"","");
		if(sendFrame(sock,tFrame) == 0)
    			printf("send successful\n");
			
	}
	
	int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
   
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( atoi(argv[2]) );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
       	//print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while(1)
    {
    	client_sock = accept(socket_desc,(struct sockaddr *)&client,
                                                            (socklen_t*)&c); 
        puts("Connection accepted");
    
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
         
      	if( pthread_create( &sniffer_thread, NULL, connection_handler, 
      							(void*) new_sock) < 0)
     	{
            perror("could not create thread");
            return 1;
        }
         
      	//Now join the thread , so that we dont terminate before the thread
       	//pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
        
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
	
	
	
	return 0;
}
/******************************************************************************
connection_hander:
	connection_handler is the thread that is created from the main function
	connection_handler holds the connection with the nodes and has the job
	of sending frames over.

******************************************************************************/ 
void *connection_handler(void *socket_desc)
{
	printf("I am now handling a client\n");
	int sock = *(int*)socket_desc;
	//sleep(5);
	int node_num=m_num;
	m_num++;
	
	//printf("Looping\n");
	char msgType[2];
	char destAddr[2];
	char srcAddr[2];
	char data[1024];
	int recvMsgSize;
	if((recvMsgSize=recv(sock,msgType,2,0))<0)
	{
		perror("recv() failed");
		exit(1);
	}
	if((recvMsgSize=recv(sock,destAddr,2,0))<0)
	{
		perror("recv() failed");
		exit(1);
	}
	int da1 = destAddr[0];
	int da2 = destAddr[1];
	if((recvMsgSize=recv(sock,srcAddr,2,0))<0)
	{
		perror("recv() failed");
		exit(1);
	}
	int sa1 = srcAddr[0];
	int sa2 = srcAddr[1];
	if((recvMsgSize=recv(sock,data,1024,0))<0)
	{
		perror("recv() failed");
		exit(1);
	}
			
	char* base = malloc(sizeof(char)*200);
	sprintf(base,"169.235.%d.%d",sa1,sa2);
	printf("\tReceived a msg type: %s\n\tFrom IP Address:"); 
    printf("%s\n\tWith data: %s\n\n\n",msgType,base,data);
			
	allAddrs[addrscount++] = sock;
			
			
			
	// now send idle_down back to the sender
	char hostname[1024];
    gethostname(hostname, 1024);
    char* basecopy = malloc(sizeof(char) * 200);
    strcpy(basecopy,base);
    //printf("My hostname: %s\n",hostname);
    char ip[100];
    hostname_to_ip(hostname , ip);
	struct Frame* myFrame = createFrame("0",basecopy,ip,"");
	printf("\tSending a message back to client\n\tmsg type:"); 
    printf("0\n\tTo IP Addr: %s\n\twith no data\n\n",base);
	if(sendFrame(sock,myFrame) == 0)
    		printf("send successful\n");
    char msgType2[2];
	char destAddr2[2];
	char srcAddr2[2];
	char data2[1024];
	int msg_type_integer;
			
	while(1)
	{
		sleep(5);
		if((recvMsgSize=recv(sock,msgType2,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		if((recvMsgSize=recv(sock,destAddr2,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		int da1 = destAddr2[0];
		int da2 = destAddr2[1];
		if((recvMsgSize=recv(sock,srcAddr2,2,0))<0)
		{
			perror("recv() failed");
			exit(1);
			}
		int sa1 = srcAddr[0];
		int sa2 = srcAddr[1];
		if((recvMsgSize=recv(sock,data2,1024,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
			
		char* base = malloc(sizeof(char)*200);
		sprintf(base,"169.235.%d.%d",sa1,sa2);
		char* newbase = malloc(sizeof(char)*200);
		sprintf(newbase,"169.235.%d.%d",da1,da2);
		printf("\tReceived a msg type: %s\n\tFrom IP Address:"); 
        printf("%s\n\tWith data: %s\n\n\n",msgType2,base,data2);
		msg_type_integer = atoi(msgType2);
		struct Frame* data_frame;
		int i, highest_req,hNum;
			
		switch(msg_type_integer)
		{
			case 1: 	

					request_arr[request_index].ip = base;
					request_arr[request_index].request_priority = 1;
				
					request_index++;
					sleep(3);
					i = 0;
					for(i; i < request_index; i++)
					{
						printf("i:%d  %d\n",i,request_arr[i].request_priority);
					}
					
					i = 0;
					highest_req = 0;
					hNum=0;
					while(1)
                    {
                        sleep(5);
						for(i; i < request_index; i++)
						{
							if(request_arr[i].request_priority > highest_req)
							{
								highest_req=request_arr[i].request_priority;
								hNum=i;
							}
						}
					    if(node_num==hNum)
					    {
						    data_frame = createFrame("3","","","");
						    if(sendFrame(sock,data_frame) == 0)
    							printf("send successful\n");
    						request_arr[highest_req].request_priority = 0;
    						break;
    					}
    					else
    					{
    						data_frame = createFrame("5","","","");
						    if(sendFrame(sock,data_frame) == 0)
    							printf("send successful\n");
    						break;
    					}
    				}
					break;
					
				
			case 2: 
					
					request_arr[request_index].ip = base;
					request_arr[request_index].request_priority = 2;
					
					request_index++;
					sleep(3);
					i = 0;
					for(i; i < request_index; i++)
					{
						printf("i: %d %d\n",i,request_arr[i].request_priority);
					}
					
					i = 0;
					highest_req = 0;
					hNum=0;
					while(1)
                    {
                        sleep(5);
					    for(i; i < request_index; i++)
					    {
						    if(request_arr[i].request_priority > highest_req)
						    {
							    highest_req = request_arr[i].request_priority;
								hNum=i;
						    }
					    }
					    if(node_num==hNum)
					    {
						    data_frame = createFrame("3","","","");
						    if(sendFrame(sock,data_frame) == 0)
    							printf("send successful\n");
    						request_arr[highest_req].request_priority = 0;
    						break;
    					}
    					else
    					{
    						data_frame = createFrame("5","","","");
						    if(sendFrame(sock,data_frame) == 0)
    							printf("send successful\n");
    						break;
    					}
    				}
    				break;


			case 4:	
                    data_frame = createFrame(msgType2,newbase,base,data2);
					int i = 0;
					for(i; i < addrscount; i++)
					{
						if(sendFrame(allAddrs[i],data_frame) == 0)
    						printf("send successful\n");
					}
					break;

			default:
				    break;
						
		}

				
			
	}
			
   	return 0;
}

