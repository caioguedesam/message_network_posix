all:
	g++ -Wall -c common.cpp
	g++ -Wall -c clientData.cpp
	g++ -Wall -c messageParser.cpp
	g++ -Wall -lpthread client.cpp common.o -o client -lpthread
	g++ -Wall -lpthread server.cpp common.o clientData.o messageParser.o -o server -lpthread