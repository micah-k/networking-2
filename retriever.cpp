#include <arpa/inet.h>    // inet_ntoa
#include <cstring>        // strstr
#include <ctype.h>        // Not sure how this got here
#include <fstream>        // ofstream
#include <iostream>       // various streams
#include <netdb.h>        // gethostbyname
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sstream>        // sstream
#include <stdio.h>        //standard I/O
#include <stdlib.h>       // standard library
#include <string>         // strings in various places
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
#define BUF_SIZE 16384
#define PORT 80
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
//
//
//

  sockaddr_in sendSockAddr;
  char* hostname = (char*)host.c_str();
  cout << "Creating socket..." << endl;
  int sd = CreateSocket(hostname, PORT, &sendSockAddr);
  if (sd == -1)
    exit(1);

  cout << "Connecting..." << endl;
  if ( connect(sd, (struct sockaddr *)&sendSockAddr, sizeof(sendSockAddr)) < 0 ) {
    close(sd);
    cout << "Could not connect" << endl;
    exit(1);
  }

	//issue GET request to server for requested file

  stringstream ssrequest;
  ssrequest << "GET " << address << " HTTP/1.0\r\n"
     << "Host: " << host << "\r\n"
     << "\r\n";
  string request = ssrequest.str();
//
//
//
	//When file is returned by server, output file to screen and file system
  cout << "Sending request..." << endl;
  if (send(sd, request.c_str(), request.length(), 0) != (int)request.length()) {
    cout << "Error sending request." << endl;
    exit(1);
  }

  /*stringstream testresponse;
  testresponse << "HTTP/1.0 200 OK\r\n" <<
    "Transfer-Encoding: chunked\r\n" <<
    "Date: Sat, 28 Nov 2009 04:36:25 GMT\r\n" <<
    "Server: LiteSpeed\r\n" <<
    "Connection: close\r\n" <<
    "X-Powered-By: W3 Total Cache/0.8\r\n" <<
    "Pragma: public\r\n" <<
    "Expires: Sat, 28 Nov 2009 05:36:25 GMT\r\n" <<
    "Etag: \"pub1259380237;gz\"\r\n" <<
    "Cache-Control: max-age=3600, public\r\n" <<
    "Content-Type: text/html; charset=UTF-8\r\n" <<
    "Last-Modified: Sat, 28 Nov 2009 03:50:37 GMT\r\n" <<
    "X-Pingback: http://net.tutsplus.com/xmlrpc.php\r\n" <<
    "Content-Encoding: gzip\r\n" <<
    "Vary: Accept-Encoding, Cookie, User-Agent\r\n" <<
    "\r\n" <<
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n" <<
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n" <<
    "<head>\n" <<
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n" <<
    "<title>Top 20+ MySQL Best Practices - Nettuts+</title>\n" <<
    "<!-- ... rest of the html ... -->\n";*/
/*
  char cur[BUF_SIZE];
  string ssresponse;
  cout << "Receiving response..." << endl;
  while(recv(sd,cur,BUF_SIZE,0)>0)
  {
    ssresponse.append(cur);
    printf("%s\n",cur);
    bzero(&cur,sizeof(cur));
  }
*/


  char cur[BUF_SIZE+1];
  stringstream ssresponse;
  stringstream docstream;
  cur[BUF_SIZE] = '\0';
  bool readingHeader = true;
  size_t readed = read(sd, &cur, BUF_SIZE);
/*  string temp = testresponse.str();
  size_t readed = temp.length();
  cout << "readed:\n" << readed << "\n";
  testresponse.read(cur, BUF_SIZE);
  printf("cur:\n%s\n", cur);*/
  while (readed > 0)
  {
    if (readingHeader)
    {
      if (readed < BUF_SIZE) {
        printf("readed < BUF_SIZE\n");
        cur[readed] = '\0';
      }
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
    printf("Reading next segment.\n");
    readed = read(sd, &cur, BUF_SIZE); 
  }

  printf("Printing ssresponse:\n");
  cout << ssresponse.str();
  printf("Outputting file:\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  cout << docstream.str();
  printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
  
//
//
//

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
    savefile << docstream;
    savefile.close();
  }
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
