#include "misc.hh"
#include <unistd.h>
#include <string>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <stdlib.h>

using namespace std;

int schdir(const char* path)
{
  if(chdir(path) < 0)
    throw runtime_error("Unable to chdir to '"+string(path)+"': "+string(strerror(errno)));
  return 0;
}

int ssystem(const char* path)
{
  if(system(path) < 0)
    throw runtime_error("Unable to execute '"+string(path)+"': "+string(strerror(errno)));
  return 0;
}

string getStringFromFP(FILE* fp)
{
  string ret;
  char buffer[4096];
  size_t len;
  string diff;
  while((len = fread(buffer, 1, sizeof(buffer), fp))) {
    ret.append(buffer, len);
  }
  return ret;
}
