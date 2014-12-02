#ifndef __NODE_H__
#define __NODE_H__

#include "BitStream.h"
#include "Frame.h"
#include "PracticalSocket.h"

#include <iostream>
#include <queue>
#include <string>
#include <exception>
#include <fstream>
#include <sstream>

#define RCVBUFSIZE 2081 // 2080 is a full frame
#define maxTHT 655 // Five AVERAGE frames (5 * SIZE_max/2), which is of course also multiple MAX frames

using namespace std;


class NodeException : public exception {
    string userMessage;
public:
    NodeException(const string &message) throw();
    ~NodeException() throw();
    const char *what() const throw();
};


class Node {
    unsigned char nodeNum; // This node's address
    unsigned int numNodes; // Total number
    bool *nodesFinished; // Will be array of finished nodes, for finish synchronization
    string servAddr;
    TCPServerSocket *myServerForLeftNode; // Need to accept a single connection
    TCPSocket *rightNode, *leftNode; // Will create sockets for each thing, won't send left or receive right
    /* UDPSocket *rightNode, *leftNode; */
    /* unsigned short rightNodePort; */
    queue<string> transmitQueue; // Queue for frames to transmit (or re-transmit)
    ifstream *inFile;
    ofstream *outFile;
    ofstream *errFile; //<<
    bool haveToken;
    BitStream leftovers; // For overflowing frames; this doesn't seem to actually happen, but it might
    void push(Frame myFrame); // Push frame onto the queue
    void push(TokenFrame myFrame);
    void push(FinishedFrame myFrame);
    Frame pop(); // Pop frame off of the queue
    Frame receiveNextFrame();
    void send(Frame myFrame); // Immediately send frame to next node
    void send(TokenFrame myframe);
    void send(FinishedFrame myframe);
    void sendToken();
    void sendFinished();
    bool isForMe(Frame recvdFrame);
    bool isFromMe(Frame recvdFrame);
    bool isMyToken(Frame recvdFrame);
    bool amMonitor(); // Lets the node know if it's the monitor, so it will do monitor node things
    bool allNodesFinished(); // For synchronizing exit
    unsigned char nodeRight(); // Right node's "address"
    unsigned char nodeLeft(); // Left node's "address"
    string generateOut(Frame myFrame); // Generate output line (separated from writing for potential debug purposes)
    string generateFullReadable(Frame myFrame);
    void writeOut(Frame myFrame); // Write output line to output file
    bool willRejectFrame(); // 20% chance
    bool wasRejected(Frame myFrame);
    bool willForwardAckNak(); // 2% chance
    bool willLoseToken(); // 5% chance, unused (token recovery unimplemented)
    bool willGarbleFrame(); // 1% chance, unuused (frame error checking unimplemented)

public:
    ~Node();
    void transmit(); // The transmit state
    void listen(); // The listen state
    void run(); // Called after initialization; returning means the node (the ring) is finished

    //Constructor
    Node(unsigned char nodeNum, unsigned int numNodes, string servAddr, unsigned short portStart) throw(NodeException);
};


#endif
