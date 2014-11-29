CXX = g++
CXXFLAGS = -Wall -g -std=c++11

LIBS = -lpthread

all: Project2

NetworkingProject2: NetworkingProject2.cpp PracticalSocket.cpp PracticalSocket.h
	$(CXX) $(CXXFLAGS) -o NetworkingProject2 NetworkingProject2.cpp PracticalSocket.cpp $(LIBS)

Project2: Project2.cpp PracticalSocket.cpp PracticalSocket.h BitStream.cpp BitStream.h Frame.cpp Frame.h Node.cpp Node.h Project_2_GenerateInput.py
	$(CXX) $(CXXFLAGS) -o Project2 Project2.cpp PracticalSocket.cpp BitStream.cpp Frame.cpp Node.cpp $(LIBS)

clean:
	$(RM) NetworkingProject2 Project2 input-file-* output-file-* error-file-*
