/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);
void tv_sub(struct  timeval *out, struct timeval *in);	                      

int main(int argc, char *argv[]) {

	int sockfd;
	float time, data_rate, throughput;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match.");
		exit(0);
	}

	sh = gethostbyname(argv[1]);       //get host's information
	if (sh == NULL) {             
		printf("error when gethostbyname");
		exit(0);
	}
	
	printf("canonical name: %s\n", sh->h_name);         //print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++) {
		printf("the aliases name is: %s\n", *pptr);
	}			
	switch(sh->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd < 0) {
		printf("error in socket");
		exit(1);
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	
	fp = fopen ("myfile.txt","r+t");
	if(fp == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}

	
	time = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);                               
	data_rate = (len/(float)time);
	throughput = data_rate*8.0/1000.0;                                    
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s), Throughput: %f (Mbps)\n", time, (int)len, data_rate, throughput);

	close(sockfd);
	fclose(fp);
	
	exit(0);
}

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len) {

	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	int flag = 0;                                //flag used to break for loop when no more data left but DUs sent less than BATCHSIZE
	ci = 0; //ci is current index
	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);   //lsize is size of entire file
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n", DATALEN);
	printf("the batch size is %d DUs\n", BATCHSIZE);
	
	buf = (char *) malloc (lsize + 1);           // allocate memory to contain the whole file (EXPLAIN the +1!!!)
	if (buf == NULL) exit (2);

	fread (buf,1,lsize,fp);                   // copy the file into the buffer.
	
	
	/*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';								//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	while(ci<= lsize) {
		for (int i = 1; i <= BATCHSIZE; i++) {
			if ((lsize+1-ci) <= DATALEN) {       //no more data left to send, set flag to 1 to break loop if i < BATCHSIZE
				slen = lsize+1-ci;
				flag = 1;
				printf("Size of Data Unit: %d Bytes\n", slen-1);
			}
			else {
				slen = DATALEN;
				printf("Size of Data Unit: %d Bytes\n", slen);
			}
			memcpy(sends, (buf+ci), slen);
			n = sendto(sockfd, &sends, slen, 0, addr, addrlen);            //send the data to server address
			if(n == -1) {
				printf("send error!");								
				exit(1);
			}
			printf("Data Unit %d sent!!!\n", i);
			ci += slen;
			if (flag) {
				break;
			}
		}
		
		n = recvfrom(sockfd, &ack, 2, 0, addr, &addrlen);    //receive data from same server address
		if (n == -1) {      
			printf("error receiving");
			exit(1);
		}
		if (ack.num != 1|| ack.len != 0) {
			printf("error in transmission\n");
		}
		else {
			printf("ACK received from server\n");
		}
	}
	
	
	gettimeofday(&recvt, NULL);
	*len= ci-1;                                                         
	tv_sub(&recvt, &sendt);                                                                 
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);

}


void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
