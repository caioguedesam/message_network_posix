SRCS=client.cpp server.cpp clientData.cpp common.cpp messageParser.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: compile clean

compile:
	g++ -Wall -c common.cpp
	g++ -Wall -c clientData.cpp
	g++ -Wall -c messageParser.cpp
	g++ -Wall -lpthread client.cpp common.o -o cliente -lpthread
	g++ -Wall -lpthread server.cpp common.o clientData.o messageParser.o -o servidor -lpthread

clean:
	rm -f $(OBJS)