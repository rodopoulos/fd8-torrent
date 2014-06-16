/*
 * System_httpServer.cpp
 *
 *  Created on: Jun 15, 2014
 *      Author: Pimenta
 */

// this
#include "System.hpp"

// standard
#include <cstring>
#include <cstdio>
#include <string>
#include <iostream>

// local
#include "Defines.hpp"
#include "Concurrency.hpp"
#include "Network.hpp"

using namespace std;
using namespace concurrency;
using namespace network;

// static variables
static TCPConnection* client = nullptr;

void System::httpServer() {
  client = httpTCPServer.accept();
  if (client == nullptr)
    return;
  
  char* data = new char[SIZE_HTTPSERVER_MAXBUF];
  memset(data, 0, SIZE_HTTPSERVER_MAXBUF);
  int tot = client->recv(SIZE_HTTPSERVER_MAXBUF, data);
  //printf("------ PACOTE ------------------------------------------\n");
  printf("total bytes request: %d\n%s\n", tot, data);
  fflush(stdout);
  
  char fn[100], buftmp[100];
  sscanf(data, "%s %s", buftmp, fn);
  //printf("\nFN: %s\n", fn);
  //printf("\nBUF: %s\n", buf);
  
  
  if (fn[1] == '?') {
    httpServer_dataRequest(fn, data);
  } else {
    if (string(fn) == "/")
      strcpy(fn, "/index.html");
    FILE* fp = fopen((string("./www") + fn).c_str(), "rb");
    if (fp) {
      if (string(fn).find(".html") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/html\r\n"
          "\r\n"
        ;
        client->send(strlen(header), header);
      }
      else if (string(fn).find(".css") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: text/css\r\n"
          "\r\n"
        ;
        client->send(strlen(header), header);
      }
      else if (string(fn).find(".js") != string::npos) {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/javascript\r\n"
          "\r\n"
        ;
        client->send(strlen(header), header);
      }
      else {
        const char* header = 
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\r"
          "Content-Type: application/octet-stream\r\n"
          "\r\n"
        ;
        client->send(strlen(header), header);
      }
      size_t total;
      while ((total = fread(data, sizeof(char), SIZE_HTTPSERVER_MAXBUF, fp)) > 0)
        client->send(total, data);
      fclose(fp);
    }
    else {
      const char* msg = "<html><body>Pagina nao encontrada.</body></html>";
      client->send(strlen(msg) + 1, msg);
    }
  }
  
  delete client;
  client = nullptr;
  delete data;
}

void System::httpServer_dataRequest(char* cRequest, const char* buffer){
  string request = string(cRequest).substr(3, strlen(cRequest));
  if (request == "host-ip"){
    string hostIP = localAddress.toString();
    client->send(hostIP.size() + 1, hostIP.c_str());
  } else if( request == "n-hosts" ){

  } else if( request == "server-state" ){
    client->send(3, "On");
  }
}
