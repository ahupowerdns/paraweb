#pragma once
#include <string>
#include <boost/format.hpp>
#include <boost/circular_buffer.hpp>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

class BufferedSocketIO
{
public:
  explicit BufferedSocketIO(int fd) : d_fd(fd), d_cb(1024)
  {
    int flags=fcntl(fd, F_GETFL,0);    
    if(flags < 0 || fcntl(fd, F_SETFL, flags|O_NONBLOCK) < 0)
      throwError("Setting non-blocked mode");
  }

  std::string getLine();
  void put(const std::string& str);

private:
  void throwError(const std::string& what) 
  {
    throw std::runtime_error(what + strerror(errno));
  }

  void waitRead()  { waitReadWrite(true);  }
  void waitWrite() { waitReadWrite(false); } 
  void waitReadWrite(bool rw);
  void readSome();

  int d_fd;
  typedef boost::circular_buffer<char> cb_t;
  cb_t d_cb;
};
