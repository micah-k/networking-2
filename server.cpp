#include <arpa/inet.h>    // inet_ntoa
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
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <sys/uio.h>      // writev
#include <sys/time.h>
#include <unistd.h>       // read, write, close


using namespace std;

struct threadData
{
  int sd;
};

#define BUF_SIZE 16384
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

void* openAndSendFile(void* whatever)
{
  

  //  If file requested does not exist, return 404 Not Found code with custom File Not Found page
  //
  //
  //
  //  If HTTP request is for SecretFile.html, return 401 Unauthorized
  //
  //
  //
  //  If request is for file that is above the directory structure where web server is running ( for example, "GET ../../../etc/passwd" ), return 403 Forbidden
  //
  //
  //
  //  if server cannot understand request return 400 Bad Request
  //
  //
  //


  threadData* data = (threadData*)whatever;
  int sd = data->sd;
  delete data;
  // After receiving GET request...
  char databuf[BUF_SIZE];
  stringstream ssrequest;

  // Repeat reading data from the client into databuf[BUF_SIZE]. 
  // Note that the read system call may return without reading 
  // the entire data if the network is slow.
  int readed = read(sd, &databuf, BUF_SIZE);

  while(readed > 0)
  {
    ssrequest.write(databuf, readed);
    readed = read(sd, &databuf, BUF_SIZE); 
  }

  printf("Printing ssrequest:\n");
  cout << ssrequest.str();

  // open file requested...

/*
  // and send with HTTP 200 OK code.
  int written = 0;
  while (written < sizeof(count))
  {
    int bytesWritten = write(sd, &count, sizeof(count));
    if (bytesWritten == -1)
    {
      printf("write failed\n");
      exit(1);
    }

    written += bytesWritten;
  }


  if (close(sd) != 0)
  {
      printf("close failed\n");
      exit(1);
  }*/
}

int main(int argc, char** argv)
{
  //wait for connection and HTTP GET request (you may do this single threaded or multi-threaded)
  IF_FALSE_RETURN(argc==2,"Improper server call.");
  int port = ConvertParameterToInt(argv[1]);



  // Declare and initialize sockaddr_in structure
  sockaddr_in acceptSockAddr;
  bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
  acceptSockAddr.sin_family      = AF_INET;
  acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  acceptSockAddr.sin_port        = htons(port);

  // Open a stream-oriented socket with the Internet address family.
  int serverSd = socket(AF_INET, SOCK_STREAM, 0);
  IF_FALSE_RETURN(serverSd != -1, "socket failed to create file descriptor");

  // Set the SO_REUSEADDR option. 
  //
  // Note: this option is useful to prompt OS to release the server port
  // as soon as your server process is terminated.
  const int on = 1;
  IF_FALSE_RETURN(setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int)) != -1, "setsockopt failed");

  // Bind this socket to its local address.
  IF_FALSE_RETURN(bind(serverSd, (sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr)) != -1, "bind failed");

  // Instruct the operating system to listen to up to 8 connection requests from clients at a time
  IF_FALSE_RETURN(listen(serverSd, 8) != -1, "listen failed");

  printf("Server: listening on port %d\n", port);

  // Receive a request from a client by calling accept that will return a new socket specific to this connection request.
  sockaddr_in newSockAddr;
  socklen_t newSockAddrSize = sizeof(newSockAddr);
  int newSd = accept(serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize);
  printf("Server: accepted connection\n");
  while (newSd != -1)
  {
    //Create a new thread
    threadData* data = new threadData();
    data->sd = newSd;

    pthread_t thread;
    if (pthread_create(&thread, NULL, openAndSendFile, data) != 0)
    {
      delete data;
      printf("pthread_create failed\n");
      return -1;
    }
    printf("Server: created thread\n");

    //After handling request, return to waiting for next request.
    
    newSd = accept(serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize);
    printf("Server: accepted another connection\n");
  }

  IF_FALSE_RETURN(newSd != -1, "accept failed");
  return 0;
}