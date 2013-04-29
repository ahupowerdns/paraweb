#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <syslog.h>
#include <errno.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <map>
#include "misc.hh"

#include <signal.h>
using namespace boost::property_tree;
using namespace boost::algorithm;
using namespace std;

static struct {
  string stagelocal;  // where our git checkout lives that we scribble & commit from
  string livelocal;   // where the git checkout lives that we publish the live site from
  string emaildestination; // where the contact form emails to
  string cookie;      // value of the cookie that confirms our login
  string pwhash;      // hash of correct admin password
  string baseurl;     // baseurl for pointing to ourselves from cgi-bin
} g_config;

void readConfig()
{
  ptree pt;
  ini_parser::read_ini(CONFIGFILE, pt);
  g_config.stagelocal=pt.get<string>("paraweb.stagelocal");
  g_config.livelocal=pt.get<string>("paraweb.livelocal");
  g_config.emaildestination = pt.get<string>("paraweb.emaildestination");
  g_config.pwhash=pt.get<string>("paraweb.pwhash");
  g_config.cookie=pt.get<string>("paraweb.cookie");
  g_config.baseurl=pt.get<string>("paraweb.baseurl");

  trim_if(g_config.stagelocal, is_any_of("\" "));
  trim_if(g_config.livelocal, is_any_of("\" "));
  trim_if(g_config.emaildestination, is_any_of("\" "));
  trim_if(g_config.pwhash, is_any_of("\" "));
  trim_if(g_config.cookie, is_any_of("\" "));
  trim_if(g_config.baseurl, is_any_of("\" "));
}


void doPatch(ptree& pt)
{
  umask(002);
  string fname = pt.get<string>("fname");
  syslog(LOG_WARNING, "Asked to apply patch '%s'", fname.c_str());

  if(fname.find_first_of("/& ;\n\r=") != string::npos) {
   syslog(LOG_ERR, "Attempt to patch file with invalid characters '%s'", fname.c_str());
   return;
  }

  string cmd="patch -s "+g_config.stagelocal+string("/")+ fname +" -";
  FILE* fp = popen(cmd.c_str(), "w");
  if(!fp)
    throw runtime_error("Unable to popen cmd '"+cmd+"': "+string(strerror(errno)));
  const string& diff = pt.get<string>("diff");
  fwrite(diff.c_str(), 1, diff.length(), fp);
  pclose(fp);

  schdir(g_config.stagelocal.c_str());

  ssystem("make publish > /dev/null 2> /dev/null");
  syslog(LOG_ERR, "Done");
  string status("Ok");
  cout << "{\"status\": \""+status+"\"}" <<endl;
}

void doGet(const string& iname)
{
  string fname(iname);
  syslog(LOG_WARNING, "Asked to serve '%s'", fname.c_str());
  
  string::size_type pos = fname.find('?');
  if(pos != string::npos)
    fname = fname.substr(0, pos);

  if(fname.find_first_of("& ;\n\r=") != string::npos || fname.find("..") != string::npos) {
   syslog(LOG_ERR, "Attempt to patch file with invalid characters '%s'", fname.c_str());
   return;
  }
  if(fname.empty())
    fname="index.html";
    
  FILE* fp = fopen((g_config.stagelocal+string("/")+fname).c_str(), "r");
  if(!fp) {
    syslog(LOG_ERR, "Unable to open file for '%s'", fname.c_str());
    return;
  }
  ptree resp;
  string content = getStringFromFP(fp);
  cout<<content;
  fclose(fp);
  return;
}


void doReset(ptree& pt)
{
  schdir(g_config.stagelocal.c_str());
  ssystem("git push > /dev/null 2> /dev/null");
  ssystem("git reset --hard HEAD > /dev/null 2> /dev/null");
  ssystem("make > /dev/null 2> /dev/null");

  string status("Ok");
  cout << "{\"status\": \""+status+"\"}" <<endl;    
}

void doDiff(ptree& pt)
{
  schdir(g_config.stagelocal.c_str());
  FILE *fp = popen("git diff", "r");
  if(!fp)
    throw runtime_error("Unable to open git diff: "+string(strerror(errno)));
  string diff=getStringFromFP(fp);
  pclose(fp);

  ptree resp;
  resp.push_back(make_pair("diff", diff));
  write_json(cout, resp);
}

void doTrigger(ptree& pt) 
{
  syslog(LOG_WARNING, "Received a trigger to rebuild");
  schdir(g_config.stagelocal.c_str());
  ssystem("git pull > /dev/null 2> /dev/null");
  ssystem("make > /dev/null 2> /dev/null");
  schdir(g_config.livelocal.c_str());
  ssystem("git pull > /dev/null 2> /dev/null");
  ssystem("make publish > /dev/null 2> /dev/null");
  string status("Ok");
  cout << "{\"status\": \""+status+"\"}" <<endl;
  syslog(LOG_WARNING, "Done with rebuild");
}

void doCommit(ptree& pt)
{
  string message = pt.get<string>("message", "none");
  schdir(g_config.stagelocal.c_str());
  FILE* fp= popen("git commit -F - -a > /tmp/blah 2> /tmp/blah2", "w");
  if(!fp)
    throw runtime_error("Unable to open git diff: "+string(strerror(errno)));  
    
  fwrite(message.c_str(), 1, message.length(), fp);
  pclose(fp);
  ssystem("git push > /tmp/one 2> /tmp/two");
  string status("Ok");
  cout << "{\"status\": \""+status+"\"}" <<endl;  
}


map<string, string> getParameters(const string& qss)
{
  map<string, string> ret;

  vector<string> pairs;
  boost::split(pairs, qss, boost::is_any_of("&"));
  BOOST_FOREACH(const string& p, pairs) {
    vector<string> kv;
    boost::split(kv, p, boost::is_any_of("="));
    ret[kv[0]]= kv.size() > 1 ? kv[1] : "";    
  }
  return ret;
}


string getContentType(const string& uri)
{
  if(boost::ends_with(uri, ".html"))
    return "text/html";
  else if(boost::ends_with(uri, ".css"))
    return "text/css";
  else if(boost::ends_with(uri, ".js"))
    return "application/javascript";
  else if(boost::ends_with(uri, ".png"))
    return "image/png";
  
  return "application/octet-stream";
}

int main(int argc, char **argv)
try
{
  if(isatty(0)) {
    cerr<<"This is a cgi-bin script which expects to be called by a webbrowser! Continuing anyhow.."<<endl;
  }
  openlog("changer", 0, LOG_MAIL);
  readConfig();

  string cookies = getenv("HTTP_COOKIE") ?: "";
  string uri = getenv("REQUEST_URI");
  string queryString = getenv("QUERY_STRING") ?: "";
  syslog(LOG_WARNING, "Changer launched: uri %s, xsrf %s, cookies %s, query %s", 
	 uri.c_str(),  getenv("HTTP_X_REQUESTED_WITH"),  cookies.c_str(), queryString.c_str());

  bool haveAuth = false;
 
  map<string, string> parameters = getParameters(queryString);

  if(cookies.find("paraweb="+g_config.cookie+";") != string::npos) 
    haveAuth = true;
  else if(parameters.count("password"))  {
    if(!strcmp(crypt(parameters["password"].c_str(), g_config.pwhash.c_str()), 
	       g_config.pwhash.c_str())) 
    {
      cout << "Set-Cookie: paraweb="<<g_config.cookie<<";path=/;Expires=Wed, 13-Jan-2021 22:23:01 GMT"<<endl;
      haveAuth=true;
    }
    else {
      haveAuth = false; 
      syslog(LOG_ERR, "No match: '%s', our output: '%s', our store: '%s'\n",
	     parameters["password"].c_str(),
	     crypt(parameters["password"].c_str(), g_config.pwhash.c_str()),
	     g_config.pwhash.c_str());
    }
  }
  
  string action;

  if(parameters.count("action"))
    action = parameters["action"];

  if(parameters.count("action"))
    action = parameters["action"];

  if(!haveAuth && action=="login") {
    cout << "Content-type: application/json" << endl<<endl;
    cout << " { \"status\": \"Incorrect credentials\" }" <<endl;
    return 0;
  }

  if(haveAuth && action == "login") {
    cout << "Content-type: application/json" << endl<<endl;
    cout << " { \"status\": \"Ok\" }" <<endl;
    return 0;
  }


  if(!haveAuth) {
    cout << "Content-type: text/html"<<endl;
    cout << "Status: 401 Unauthorized"<<endl<<endl;
    cout << "<html><head><script>window.location='"+g_config.baseurl+"/login.html?goal="<<uri<<"';</script></head></html>"<<endl;
    return 0;
  }
  ///////////// authorized from this point on!

  string::size_type pos = uri.find("cgi-bin/changer/");  // if final slash: serve up page
  if(pos != string::npos && uri.find("cgi-bin/changer/cgi-bin/changer") == string::npos) {
    string contentType = getContentType(uri);
    
    cout << "Content-type: " << contentType << endl<<endl;
    doGet(uri.substr(pos+16));
    return 0;
  }
  
  ptree pt;
  if(action.empty()) {
    try {
      read_json(cin, pt);
      action = pt.get<string>("action", "none");
      if(!getenv("HTTP_X_REQUESTED_WITH") || strcmp(getenv("HTTP_X_REQUESTED_WITH"), "XMLHttpRequest")) {
	syslog(LOG_ERR, "Cross site request forgery attempt, referer = %s", getenv("HTTP_REFERER"));
	return 0;
      }      
    }
    catch(...) {
      action = "trigger";
    }
  }
  syslog(LOG_WARNING, "Action = '%s'", action.c_str());
  cout << "Content-type: application/json" << endl<<endl;
  
  if(action == "patch") {
    doPatch(pt);
  }  
  else if(action == "get") {
    doGet(pt.get<string>("fname"));
  } else if(action == "diff") {
    doDiff(pt);
  } else if(action == "commit") {
    doCommit(pt);
  } else if(action == "reset") {
    doReset(pt);
  } else if(action == "trigger") {
    doTrigger(pt);
  }
  
  return 0;
}
catch(exception& e)
{
  syslog(LOG_ERR, "Caught fatal exception: %s", e.what());
  return 1;
}
