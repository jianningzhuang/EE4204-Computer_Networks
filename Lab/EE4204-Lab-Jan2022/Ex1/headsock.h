#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define MAXSIZE 50


gcc udp_client.c -o udp_client

gcc udp_ser.c -o udp_ser

./udp_ser

./udp_client

diff myfile.txt myTCPreceive.txt