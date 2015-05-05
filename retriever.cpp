#include <arpa/inet.h>    // inet_ntoa
#include <cstring>        // strstr
#include <ctype.h>        // Not sure how this got here
#include <fstream>        // ofstream
#include <iostream>       // various streams
#include <netdb.h>        // gethostbyname
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sstream>        // sstream
#include <stdio.h>        // standard I/O
#include <stdlib.h>       // standard library
#include <string>         // strings in various places
#include <strings.h>      // bzero
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <sys/time.h>
#include <sys/types.h>    // socket, bind
#include <sys/uio.h>      // writev
#include <unistd.h>       // read, write, close

using namespace std;

#define BUF_SIZE 16384
#define PORT 80

//
// Macros to convieniently add basic error handling
//
#define IF_FALSE_RETURN_VAL(test, val) \
  if (!(test)) \
  { \
    return val; \
  }

#define IF_FALSE_FAIL_VAL(test, val, msg) \
  if (!(test)) \
  { \
    printf("%s\n", msg); \
    return val; \
  }

#define IF_FALSE_CLOSE_AND_FAIL_VAL(test, sd, val, msg) \
  if (!(test)) \
  { \
    printf("%s\n", msg); \
    close(sd); \
    return val; \
  }

// CreateSocket
//
// Creates socket file descriptor and sockaddr_in
//
int CreateSocket(char* name, int port, sockaddr_in* sockAddr)
{
  struct hostent* host = gethostbyname( name );
  IF_FALSE_FAIL_VAL(host != NULL, -1, "400 Bad Request");

  bzero( (char*)sockAddr, sizeof( sockAddr ) );

  sockAddr->sin_family = AF_INET;
  sockAddr->sin_addr.s_addr = inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
  sockAddr->sin_port = htons( port );

  int sd = socket( AF_INET, SOCK_STREAM, 0 );
  IF_FALSE_FAIL_VAL(sd != -1, -1, "socket failed to create file descriptor");
  return sd;
}

int main(int argc, char** argv)
{
  //take input from command line

  //Make sure we got the right number of parameters or display usage
  IF_FALSE_FAIL_VAL(argc == 2, 1, "400 Bad Request");

  //parse server address and file requested

  string host;
  string address = argv[1];
  size_t slashpos = address.find("/");
  if(slashpos != string::npos)
  {
    host = address.substr(0, slashpos);
    address = address.substr(slashpos);
  } else {
    host = address;
    address = "/";
  }

  cout << "host:" << host << "\r\n";
  cout << "address:" << address << "\r\n";

  int port = 80;
  if(host == "127.0.0.1")
  {
    port = 9649;
  }

  sockaddr_in sendSockAddr;
  char* hostname = (char*)host.c_str();
  cout << "Creating socket..." << endl;
  int sd = CreateSocket(hostname, port, &sendSockAddr);
  IF_FALSE_RETURN_VAL(sd != -1, 1);

  cout << "Connecting..." << endl;
  int result = connect(sd, (struct sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
  IF_FALSE_CLOSE_AND_FAIL_VAL(result >= 0, sd, 1, "Could not connect");

	//issue GET request to server for requested file

  stringstream ssrequest;
  ssrequest << "GET " << address << " HTTP/1.0\r\n"
     << "Host: " << host << "\r\n"
     << "\r\n";
  string request = ssrequest.str();

  cout << "Sending request..." << endl;
  result = send(sd, request.c_str(), request.length(), 0);
  IF_FALSE_CLOSE_AND_FAIL_VAL(result == (int)request.length(), sd, 1, "Error sending request.");

  //read and parse server response
  char cur[BUF_SIZE+1];
  bzero(&cur,sizeof(cur));

  stringstream ssresponse;
  stringstream docstream;

  bool readingHeader = true;

  size_t readed = read(sd, &cur, BUF_SIZE);
  while (readed > 0)
  {
    if (readingHeader)
    {
      char* headerEnd = strstr(cur, "\r\n\r\n");
      printf("headerEnd:\n%s\n", headerEnd);
      if (headerEnd != NULL)
      {
        int headerSize = headerEnd-cur+4;
        printf("headerSize:\n%i\n", headerSize);
        ssresponse.write(cur, headerSize);
        docstream.write(headerEnd+4, readed-headerSize);
        readingHeader = false;
        printf("Changed from header to doc.\n");
      }
      else
      {
        ssresponse.write(cur, readed);
        printf("Writing to ssresponse.\n");
      }
    }
    else 
    {
      docstream.write(cur, readed);
      printf("Writing to docstream.\n");
    }

    bzero(&cur,sizeof(cur));

    printf("Reading next segment.\n");
    readed = read(sd, &cur, BUF_SIZE); 
  }

  close(sd);

  //When file is returned by server, output file to screen and file system

  printf("Printing ssresponse:\n");
  cout << ssresponse.str();
  printf("Outputting file:\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  cout << docstream.str();
  printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  
	//If server returns error code instead of OK code, do not save file; display on the screen whatever error page was sent with error

  int httpCode = atoi(ssresponse.str().substr(9,3).c_str());
  cout << "HTTP code " << httpCode << endl << endl;
  if(httpCode == 200)
  {
    int lastdot = address.find_last_of(".");
    string filetype;
    if(lastdot > address.length())
    {
      filetype = ".html";
    }
    else
    {
      filetype = address.substr(lastdot);
    }

    ofstream savefile(("file" + filetype).c_str());
    savefile << docstream.str();
    savefile.close();
  }

	//exit after receiving response

  return 0;
}
