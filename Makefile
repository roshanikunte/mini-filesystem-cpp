CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -pthread

all: fsserver fsclient

fsserver: src/server.cpp src/filesystem.cpp src/persistence.cpp include/filesystem.hpp include/persistence.hpp
	$(CXX) $(CXXFLAGS) src/server.cpp src/filesystem.cpp src/persistence.cpp -o fsserver

fsclient: src/client.cpp
	$(CXX) $(CXXFLAGS) src/client.cpp -o fsclient

clean:
	rm -f fsserver fsclient *.o
