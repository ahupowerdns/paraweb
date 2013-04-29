#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <signal.h>
#include "socketio.hh"

using namespace boost::property_tree;
using namespace std;

string getsmtpline(BufferedSocketIO& bsi)
{
  string ret, line;
  do {
    line = bsi.getLine();
    ret.append(line);
  }
  while(line.size() > 3 && line[3]=='-');
  if(ret.empty() || (ret[0]!='2' && ret[0]!='3'))
    throw runtime_error("Wrong SMTP error code");
  return ret;
}

void doEmail(ptree& pt)
{
  string email = pt.get<string>("email", "none");
  string query = pt.get<string>("query", "none");
  string organization = pt.get<string>("organization", "none");
  string phone = pt.get<string>("phone", "none");
  string name = pt.get<string>("name", "none");
  string subject = pt.get<string>("subject", "none");
  unsigned long obfuscated = pt.get<unsigned long>("obfuscated", 0);

  if(obfuscated != 3551233UL*3551233UL) {
    syslog(LOG_ERR, "Incorrect obfuscated code %ld", obfuscated);
    exit(EXIT_FAILURE);
  }

  if(email.find_first_of("\r\n") != string::npos || subject.find_first_of("\r\n") != string::npos) {
    syslog(LOG_ERR, "Subject or email contained illegal characters");
    exit(EXIT_FAILURE);
  }

  ptree ptconf;
  ini_parser::read_ini(CONFIGFILE, ptconf);
  string emaildestination = pt.get<string>("paraweb.emaildestination");

  signal (SIGPIPE, SIG_IGN);
  struct sockaddr_in remote;
  remote.sin_family = AF_INET;
  remote.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  remote.sin_port = htons(25);
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  if(sock < 0 || connect(sock, (struct sockaddr*) &remote, sizeof(remote)) < 0) {
    syslog(LOG_ERR, "Socket error: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
 
  string status;
  try {
    BufferedSocketIO bsi(sock);
    string line = getsmtpline(bsi);

    bsi.put("EHLO dan\r\n");
    line = getsmtpline(bsi);
    bsi.put("MAIL From:<"+emaildestination+">\r\n");
    line = getsmtpline(bsi);
    bsi.put("RCPT To:<"+emaildestination+">\r\n");
    line = getsmtpline(bsi);
    bsi.put("DATA\r\n");
    line = getsmtpline(bsi);

    bsi.put("From: "+emaildestination+"\r\n");
    bsi.put("To: "+emaildestination+"\r\n");
    bsi.put("Cc: "+email+"\r\n");
    bsi.put("Subject: PowerDNS Information Request: "+subject+"\r\n\r\n");
    // insert date, message-id

    string message = 
      (boost::format("Contact form submission from %s <%s>\nPhone: %s\nOrganization: '%s'\n") %
       name % email % phone % organization).str();
    message += "Subject: "+subject+"\n";

    message += (boost::format("IP Address: %s\nUser Agent: %s\nLanguages: %s\n\n")
      % getenv("REMOTE_ADDR") % getenv("HTTP_USER_AGENT") %getenv ("HTTP_ACCEPT_LANGUAGE")).str();
    
    message += "The message: \n\n  ---- Message begins ----\n";
    message += query;
    message += "\n\n ---- Message ends ----\n";

    boost::replace_all(message, "\r\n.\r\n", "\r\n..\r\n");
    boost::replace_all(message, "\n.\n", "\n..\n");
    message += "\r\n.\r\n";

    bsi.put(message);

    line = getsmtpline(bsi);
    close(sock);

    int res = atoi(line.c_str());

    if(res == 250)
      status = "Thank you, your message has been sent to "+emaildestination;
    else {
      syslog(LOG_ERR, "Failed to send message: proper exit=%s", line.c_str()); 
      status = "There was an error sending your message";
    }
  }
  catch(exception& e) {
    status = e.what();
  }
  cout << "{\"status\": \""+status+"\"}" <<endl;

  syslog(LOG_WARNING, "Requested to send out email to %s for %s, status: %s", emaildestination.c_str(), 
	 email.c_str(), status.c_str());  
}

int main(int argc, char **argv)
try
{
  struct rlimit rlim;
  rlim.rlim_cur=50000000;
  rlim.rlim_max=50000000;
  setrlimit(RLIMIT_AS, &rlim); 
  openlog("mailer", 0, LOG_MAIL);

  ptree pt;
  read_json(cin, pt);
  string action=pt.get<string>("action");
  
  cout << "Content-type: application/json" << endl<<endl;
  
  if(action == "email") {
    doEmail(pt);
  }
  
  return 0;
}
catch(exception&e )
{
  syslog(LOG_ERR, "Caught fatal exception: %s", e.what());
  return 1;
}
