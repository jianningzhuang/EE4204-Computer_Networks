/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

void str_ser(int sockfd);                                                        

int main(int argc, char *argv[]) {

	int sockfd, ret;
	struct sockaddr_in my_addr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                                     //create socket
	if (sockfd < 0) {			
		printf("error in socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));   //bind socket
	if (ret < 0) {           
		printf("error in binding");
		exit(1);
	}
	while (1) {
		printf("Waiting for data...\n");
		str_ser(sockfd);
	}
	
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd) {
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, addrlen, n = 0;
	long lseek=0;
	struct sockaddr_in cli_addr;
	addrlen = sizeof (struct sockaddr_in);
	end = 0;
	
	while(!end) {
		for (int i = 1; i <= BATCHSIZE; i++) {
			n = recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&cli_addr, &addrlen);
			if (n ==-1) {
				printf("error when receiving\n");
				exit(1);
			}
			printf("Data Unit %d received!!!\n", i);
			if (recvs[n-1] == '\0') {
				end = 1;
				n--;
			}
			memcpy((buf+lseek), recvs, n);
			printf("Size of Data Unit: %d Bytes\n", n);
			lseek += n;
			if (end) {
				break;
			}
		}
		ack.num = 1;
		ack.len = 0;
		n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&cli_addr, addrlen);
		if (n ==-1) {
			printf("send error!");								//send the ack
			exit(1);
		}
		printf("ACK sent to client\n");
	}

	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}



