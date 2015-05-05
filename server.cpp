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
#define PORT 9649
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

  printf("Begin thread:\n");
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
    printf("Reading data...\n");
    ssrequest.write(databuf, readed);
    if(readed >= BUF_SIZE)
    {
      readed = read(sd, &databuf, BUF_SIZE);
    }
    else
    {
      readed = 0;
    }
  }

  printf("Printing ssrequest:\n");
  cout << ssrequest.str();

  // open file requested...
  int httpcode;
  ifstream file();
  string filerequest = ssrequest.str();
  filerequest = filerequest.erase(0, 4);
  filerequest = filerequest.substr(0, filerequest.find(" "));
  ifstream file(filerequest);
  if(filerequest == "SecretFile.html")
  {
    httpcode = 401;
    file.close();
  }
  else if(strstr("..")!=NULL)
  {
    httpcode = 403;
    file.close();
  }
  else if(!file.good())
  {
    httpcode = 404;
    file.close();
  }
  else
  {
    httpcode = 200;
  }

  stringstream ssresponse;
  ssresponse << "HTTP/1.0 " << httpcode;
  switch(httpcode)
  {
  case 401:
    ssresponse << "Unauthorized\r\n\r\n";
    break;
  case 403:
    ssresponse << "Forbidden\r\n\r\n";
    break;
  case 404:
    ssresponse << "File Not Found\r\n\r\n";
    break;
  default:
    ssresponse << "OK\r\n\r\n";
    break;
  }

  if(httpcode==200)
  {
    ssresponse << file;
  }
  else
  {
    ssresponse << "<p>" << httpcode << "</p>";
  }

  string response = ssresponse.str();

  // and send with HTTP 200 OK code.
  cout << "Sending response..." << endl;
  if (send(sd, response.c_str(), response.length(), 0) != (int)response.length()) {
    cout << "Error sending response." << endl;
    exit(1);
  }

  if (close(sd) != 0)
  {
      printf("close failed\n");
      exit(1);
  }
}

int main(int argc, char** argv)
{
  //wait for connection and HTTP GET request (you may do this single threaded or multi-threaded)
  



  // Declare and initialize sockaddr_in structure
  sockaddr_in acceptSockAddr;
  bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
  acceptSockAddr.sin_family      = AF_INET;
  acceptSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  acceptSockAddr.sin_port        = htons(PORT);

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

  printf("Server: listening on port %d\n", PORT);

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