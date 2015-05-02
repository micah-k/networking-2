#include <arpa/inet.h>    // inet_ntoa
#include <cstring>
#include <ctype.h>
#include <fstream>
#include <iostream>
#include <netdb.h>        // gethostbyname
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <sys/time.h>
#include <sys/types.h>    // socket, bind
#include <sys/uio.h>      // writev
#include <unistd.h>       // read, write, close

using namespace std;

//
// IF_FALSE_RETURN
//
// Macro to convieniently add basic error handling
//
#define IF_FALSE_RETURN(test, msg) \
  if (!(test)) \
  { \
    printf("%s\n", msg); \
    return -1; \
  }

// ConvertParameterToInt
//
// several of our parameters are expected to be well formed numbers
// since no negative values are expected, return -1 to indicate an error.
//
int ConvertParameterToInt(char* value)
{
  char* endptr = NULL;
  long int result = strtol(value, &endptr, 0);
  if (*value == '\0' || *endptr != '\0')
    return -1;

  return (int)result;
}

// CreateSocket
//
// Creates socket file descriptor and sockaddr_in
//
int CreateSocket(char* name, int port, sockaddr_in* sockAddr)
{
  struct hostent* host = gethostbyname( name );
  IF_FALSE_RETURN(host != NULL, "400 Bad Request");

  bzero( (char*)sockAddr, sizeof( sockAddr ) );

  sockAddr->sin_family = AF_INET;
  sockAddr->sin_addr.s_addr = inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
  sockAddr->sin_port = htons( port );

  int sd = socket( AF_INET, SOCK_STREAM, 0 );
  IF_FALSE_RETURN(sd != -1, "socket failed to create file descriptor");
  return sd;
}

int main(int argc, char** argv)
{
  //take input from command line

  //Make sure we got the right number of parameters or display usage
  if (argc != 2)
  {
    printf("400 Bad Request");
    exit(1);
  }

  //parse server address and file requested

  char* temp = argv[1];
  char* serverIp = strtok(temp,"/");
  char* file = strtok(NULL,"\0");

  if(host==NULL||h->h_addr == NULL)
  {
    printf("400 Bad Request");
    exit(1);
  }
//
//
//

  sockaddr_in sendSockAddr;
  int sd = CreateSocket(serverIp, port, &sendSockAddr);
  if (sd == -1)
    exit(1);

  if ( connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0 ) {
    close(sock);
    cout << "Could not connect" << endl;
    exit(1);
  }

	//issue GET request to server for requested file

  stringstream ssrequest;
  ssrequest << "GET " << file << " HTTP/1.1\r\n"
     << "Host: " << serverIp << "\r\n"
     << "\r\n\r\n";
  string request = ssrequest.str();
//
//
//
	//When file is returned by server, output file to screen and file system

  if (send(sock, request.c_str(), request.length(), 0) != (int)request.length()) {
    cout << "Error sending request." << endl;
    exit(1);
  }

  stringstream ssresponse;

  char cur;
  while ( read(sock, &cur, 1) > 0 ) {
    ssresponse << cur;
  }

  cout << ssresponse;
//
//
//
	//If server returns error code instead of OK code, do not save file; display on the screen whatever error page was sent with error
//
//
//
	//exit after receiving response
//
//
//
/*
  // Open a new socket and establish a connection to a server.
  sockaddr_in sendSockAddr;
  int sd = CreateSocket(serverIp, port, &sendSockAddr);
  if (sd == -1)
    return 1;

  IF_FALSE_RETURN(connect(sd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == 0, "connect failed");

  //Allocate databuf[nbufs][bufsize].
  char databuf[nbufs][bufsize];
  FillBuffer((char*)databuf, nbufs * bufsize);

  struct timeval start;
  struct timeval lap;
  struct timeval end;

  // Start a timer by calling gettimeofday.
  gettimeofday(&start, NULL);

  // Repeat the repetition times of data transfers,
  for (int reps = 0; reps < repetition; reps++)
  {
    // each based on type such as 1: multiple writes, 2: writev, or 3: single write
    switch(type)
    {
    case 1: // multiple writes
      {
        for (int j = 0; j < nbufs; j++)
          IF_FALSE_RETURN(write(sd, databuf[j], bufsize) != -1, "write failed");
      }
      break;
    case 2: // writev
      {
        struct iovec vector[nbufs];
        for (int j = 0; j < nbufs; j++)
        {
          vector[j].iov_base = databuf[j];
          vector[j].iov_len = bufsize;
        }
        IF_FALSE_RETURN(writev(sd, vector, nbufs) != -1, "writev failed");
      }
      break;
    case 3: // single write
      IF_FALSE_RETURN(write(sd, databuf, nbufs * bufsize) != -1, "write failed");
      break;
    }
  }

  // Lap the timer by calling gettimeofday, where lap - start = data-sending time.
  gettimeofday(&lap, NULL);

  // Receive from the server an integer acknowledgement that shows how many times the server called read( ).
  int count = 0;
  int reads = 0;
  while (reads < sizeof(count))
  {
    int bytesRead = read(sd, &count, sizeof(count));
    IF_FALSE_RETURN(bytesRead != -1, "read failed");
    reads += bytesRead;
  }

  // Stop the timer by calling gettimeofday, where stop - start = round-trip time.
  gettimeofday(&end, NULL);

  // Print out the statistics as shown below:
  printf("Test %d: data-sending time = %ld usec, round-trip time = %ld usec, #reads = %d\n",
    testno,
    (lap.tv_sec - start.tv_sec) * 1000000 + (lap.tv_usec - start.tv_usec),
    (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec),
    count);

  // Close the socket
  IF_FALSE_RETURN(close(sd) == 0, "close failed");*/

  return 0;
}