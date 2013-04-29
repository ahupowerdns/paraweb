#include "socketio.hh"

#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

void BufferedSocketIO::waitReadWrite(bool rw)
{
  struct pollfd pfd;
  memset(&pfd, 0, sizeof(pfd));
  pfd.fd = d_fd;
  
  if(rw)
    pfd.events=POLLIN;
  else
    pfd.events=POLLOUT;

  if(poll(&pfd, 1, -1) < 0)
    throwError("Waiting for I/O opportunity");
}

string BufferedSocketIO::getLine()
{
  string ret;
  cb_t::const_iterator iter;
  char c;

  for(;;) {
    while(!d_cb.empty()) {
      c = d_cb[0];
      d_cb.pop_front();
      ret.append(1, c);
      if(c=='\n')
	return ret;
    }
    waitRead();
    readSome();
  }
}

void BufferedSocketIO::readSome()
{
  int toRead = d_cb.capacity() - d_cb.size();
  char buffer[toRead];
  int ret = read(d_fd, buffer, toRead);
  if(ret < 0)
    throwError("Reading data from socket");

  // unsure how we deal with EOF
  for(int n=0; n < ret; ++n)
    d_cb.push_back(buffer[n]);
}

void BufferedSocketIO::put(const std::string& str)
{
  string::size_type pos = 0;
  for(;;) {
    waitWrite();
    int ret = write(d_fd, str.c_str() + pos, str.size() - pos);
    if(ret < 0)
      throwError("Writing to socket");
    if(ret == 0)
      throw runtime_error("EOF writing!");
    if((string::size_type)ret == str.size() - pos)
      break;
    pos += ret;
  }

}
