// Copyright 2011, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold <buchholb>

#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <string>
#include <sstream>
#include <iostream>

#include "./Log.h"

using std::string;

namespace ad_utility {
static const int MAX_NOF_CONNECTIONS = 20;
static const int RECIEVE_BUFFER_SIZE = 10000;


//! Basic Socket class used by the server code of the semantic search.
//! Wraps low-level socket calls should possibly be replaced by
//! different implementations and wrap them instead.
class Socket {
public:

  // Default ctor.
  Socket() : _fd(-1) {
    _buf = new char[RECIEVE_BUFFER_SIZE];
  }

  // Destructor, close the socket if open.
  ~Socket() {
    ::close(_fd);
    delete[] _buf;
  }

  int close() {
    return ::close(_fd);
  }

  //! Create the socket.
  bool create(bool useTcpNoDelay = false) {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (useTcpNoDelay) {
      int flag = 1;
      int result = setsockopt(_fd,            /* socket affected */
                              IPPROTO_TCP,     /* set option at TCP level */
                              TCP_NODELAY,     /* name of option */
                              (char*) &flag,  /* the cast is historical
                                                         cruft */
                              sizeof(int));    /* length of option value */
      if (result < 0) return false;
    }
    makeResusableAfterClosing();
    return isOpen();
  }

  void setKeepAlive(bool keepAlive) const {
    int keepAliveVal = (keepAlive) ? 1 : 0;
    int rc = setsockopt(_fd, SOL_SOCKET,
        SO_KEEPALIVE, &keepAliveVal, sizeof(keepAliveVal));
    if (rc < 0) {
      LOG(WARN) << "setsockopt(SO_KEEPALIVE) failed" << std::endl;
    }
  }

  void makeResusableAfterClosing() const {
    // Make sockets reusable immediately after closing
    int on = 1;
    int rc = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (rc < 0) {
      LOG(WARN) << "! WARNING: setsockopt(SO_REUSEADDR) failed" << std::endl;
    }
  }

  //! Bind the socket to the given port.
  bool bind(const int port) {
    if (!isOpen()) return false;
    struct addrinfo hints;
    struct addrinfo* res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    std::ostringstream os;
    os << port;
    getaddrinfo(NULL, os.str().c_str(), &hints, &res);

    // bind it to the port we passed in to getaddrinfo():
    return ::bind(_fd, res->ai_addr, res->ai_addrlen) != -1;
  }

  //! Make it a listening socket.
  bool listen() const {
    if (!isOpen()) return false;
    return ::listen(_fd, MAX_NOF_CONNECTIONS) != -1;
  }

  //! Accept a connection.
  bool acceptClient(Socket* client) {
    struct sockaddr_storage clientAddr;
    socklen_t addrSize;
    addrSize = sizeof(clientAddr);
    client->_fd = ::accept(_fd, (struct sockaddr*) &clientAddr, &addrSize);
    return client->isOpen();
  }

  //! State if the socket's file descriptor is valid.
  bool isOpen() const {
    return _fd != -1;
  }

  //! Send some string.
  int send(const std::string& data) const {
    LOG(TRACE) << "Called send() ... data: " << data << std::endl;
    LOG(TRACE) << "data.size(): " << data.size() << std::endl;
    return send(data.c_str(), data.size(), 5);
  }

  //! Send some string.
  int send(const char* data, size_t nofBytes, int timesRetry) const {
    int nb = ::send(_fd, data, nofBytes, MSG_NOSIGNAL);
    if (nb != static_cast<int>(nofBytes)) {
      LOG(DEBUG) << "Could not send as much data as intended." << std::endl;
      if (nb == -1) {
        LOG(DEBUG) << "Errno: " << errno << std::endl;
        if (errno == 11 && timesRetry > 0) {
          LOG(DEBUG) << "Retrying " << timesRetry-- << " times " << std::endl;
          return send(data, nofBytes, timesRetry);
        }
      } else {
        LOG(DEBUG) << "Nof bytes sent: " << nb << std::endl;
        LOG(DEBUG) << "Calling continuation..." << std::endl;
        return nb + send(data + nb, nofBytes - nb, 5);
      }
    }
    return nb;
  }

  void getHTTPRequest(string& req, string& headers) const {
    std::ostringstream os;
    req.clear();
    headers.clear();
    for (;;) {
      auto rv = recv(_fd, _buf, RECIEVE_BUFFER_SIZE - 1, MSG_DONTWAIT);
      if (rv == 0) { break; }
      if (rv == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          LOG(WARN) << "Error during recv, errno: " << errno << std::endl;
          break;
        }
        continue;
      } else {
        // Append
        _buf[rv] = '\0';
        os << _buf;
        if (req.size() == 0) {
          auto posCRLF = os.str().find("\r\n");
          if (posCRLF != string::npos) {
            req = os.str().substr(0, posCRLF);
            os.str(os.str().substr(posCRLF));
          }
        }
        if (req.size() > 0) {
          if (req.find("HTTP") == string::npos) {
            LOG(WARN) << "Discarding invalid request: " << req << std::endl;
            return;
          }
          auto posDoubleCRLF = os.str().find("\r\n\r\n");
          if (posDoubleCRLF != string::npos) {
            headers == os.str();
            break;
          }
        }
      }
    }
  }

  // Copied from online sources. Might be useful in the future.
//    void setNonBlocking(const bool val)
//    {
//      int opts = fcntl(_fd, F_GETFL);
//      if (opts < 0)
//      {
//        return;
//      }
//
//      if (val) opts = (opts | O_NONBLOCK);
//      else opts = (opts & ~O_NONBLOCK);
//
//      fcntl(_fd, F_SETFL, opts);
//    }

private:
  int _fd;
  char* _buf;;
};
}


