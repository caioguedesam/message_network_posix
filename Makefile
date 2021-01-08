all:
	g++ -Wall -c common.cpp
	g++ -Wall -c clientData.cpp
	g++ -Wall client.cpp common.o -o client
	g++ -Wall -lpthread server.cpp common.o clientData.o -o server -lpthread