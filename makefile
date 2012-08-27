INCLS = -I.
CXX      = g++
CXXFLAGS = -O2 -s -pedantic -Wall -Wextra -c -Wno-deprecated -std=c++0x -DNDEBUG $(INCLS)
LINK     = -lpthread
PROGRAM  = ./simplest_http_server
SOURCES  = $(shell find . -iname '*.cpp')
OBJS    = $(subst .cpp,.o,$(SOURCES))

.SUFFIXES: .o .cpp

all: $(OBJS)
	$(CXX) $(notdir $(OBJS)) -o $(PROGRAM) $(LINK)

-include depend.mak

depend:
	g++ -MM $(SOURCES) > depend.mak

.cpp.o:
	$(CXX) $(CXXFLAGS) $(COMPILE) $<

clean:
	rm -rf $(notdir $(OBJS)) depend.mak $(PROGRAM)

run: all
	./simplest_http_server /var/www/ 8080 20 8 > /dev/null 2>&1 &

stop:
	killall simplest_http_server

test: run
	ab -r -c 1000 -n 300000 http://127.0.0.1:8080/index.html
	killall simplest_http_server