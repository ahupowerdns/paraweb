all: mailer changer

INCLUDEPATH=../
-include ../website.conf
-include *.d


CPPFLAGS += -MMD -O -g -Wall $(CGIOPTIONS) -DCONFIGFILE=\"${CONFIGFILE}\"

clean:
	rm -f *.o *~ mailer changer

mailer: mailer.o socketio.o misc.o
	g++ mailer.o socketio.o misc.o -static -o mailer

changer: changer.o misc.o
	g++ changer.o misc.o -static -o changer -lcrypt


dist: all
	strip mailer changer
	rsync -z mailer ${WWWLIVE}/cgi-bin/
	rsync -z changer mailer ${WWWSTAGE}/cgi-bin/	
