#include "Node.h"
#include "BitStream.h"
#include "Frame.h"
#include "PracticalSocket.h"

#include <iostream>
#include <queue>
#include <unistd.h>

using namespace std;


// NodeException

NodeException::NodeException(const string &message) throw() : userMessage(message)
{
    
}

NodeException::~NodeException() throw()
{
    
}

const char *NodeException::what() const throw()
{
    return userMessage.c_str();
}


// Node


void Node::push(Frame myFrame)
{
    *errFile << to_string((int) nodeNum) << " pushed new frame (" << generateFullReadable(myFrame) << ") onto the queue" << endl; //<<
    transmitQueue.push(myFrame.contentsAsStream().contentString());
}

void Node::push(TokenFrame myFrame)
{
    Frame realFrame(myFrame.contentsAsStream());
    *errFile << to_string((int) nodeNum) << " pushed new token frame (" << generateFullReadable(realFrame) << ") onto the queue" << endl; //<<
    transmitQueue.push(myFrame.contentsAsStream().contentString());
}

void Node::push(FinishedFrame myFrame)
{
    Frame realFrame(myFrame.contentsAsStream());
    *errFile << to_string((int) nodeNum) << " pushed new finished frame (" << generateFullReadable(realFrame) << ") onto the queue" << endl; //<<
    transmitQueue.push(myFrame.contentsAsStream().contentString());
}

Frame Node::pop()
{
    Frame *newFrame;
    if (transmitQueue.empty()) {
        *errFile << to_string((int) nodeNum) << " tried to pop from the queue when it was empty!" << endl; //<<
        newFrame = new Frame();
    } else {
        BitStream newFrameStream(transmitQueue.front());
        newFrame = new Frame(newFrameStream);
        transmitQueue.pop();
    }
    *errFile << to_string((int) nodeNum) << " popped frame (" << generateFullReadable(*newFrame) << ") off of the queue" << endl; //<<
    return *newFrame;
}

Frame Node::receiveNextFrame()
{
    Frame *nextFrame = NULL;
    char myBuf[RCVBUFSIZE] = {'\0'};
    int recvdBits = 0;
    if (leftovers.numBytes() > 0) {
        *errFile << to_string((int) nodeNum) << " has leftovers (" << leftovers.contentString() << ")" << endl; //<<
        nextFrame = new Frame(leftovers);
        if (!nextFrame->isIncomplete()) {
            *errFile << to_string((int) nodeNum) << " has complete frame from leftovers (" << generateFullReadable(*nextFrame) << ")" << endl; //<<
            leftovers = nextFrame->leftovers();
            return *nextFrame;
        }
    }
    if((recvdBits = leftNode->recv(myBuf, RCVBUFSIZE - 1)) > 0) {
        BitStream nextStream(recvdBits, myBuf);
        *errFile << to_string((int) nodeNum) << " received raw frame " << nextStream.contentString() << endl;
        nextFrame = new Frame(leftovers + nextStream);
        leftovers = nextFrame->leftovers();
        return *nextFrame;
    }
    // string recvdFromAddr;
    // unsigned short recvdFromPort;
    // if((recvdBits = leftNode->recvFrom(myBuf, RCVBUFSIZE - 1, recvdFromAddr, recvdFromPort)) > 0) {
    //     BitStream nextStream(recvdBits, myBuf);
    //     *errFile << to_string((int) nodeNum) << " received raw frame " << nextStream.contentString() << " from " << recvdFromAddr << ":" << to_string(recvdFromPort) << endl;
    //     nextFrame = new Frame(leftovers + nextStream);
    //     leftovers = nextFrame->leftovers();
    //     return *nextFrame;
    // }

    nextFrame = new Frame();
    return *nextFrame;
}

void Node::send(Frame myFrame)
{
    string frameString = myFrame.contentsAsStream().contentString();
    const char *frameCString = frameString.c_str();
    int frameCStringLen = frameString.length();
    rightNode->send(frameCString, frameCStringLen);
    // rightNode->sendTo(frameCString, frameCStringLen, servAddr, rightNodePort);
    *errFile << to_string((int) nodeNum) << " sent new frame " << generateFullReadable(myFrame) << " (from " << (int) myFrame.sourceAddress() << ") to " << (int) myFrame.destAddress() << endl; //<<
    *errFile << to_string((int) nodeNum) << " sent new raw frame " << frameString << endl;
}

void Node::send(TokenFrame myFrame)
{
    Frame realFrame(myFrame.contentsAsStream());
    send(realFrame);
}

void Node::send(FinishedFrame myFrame)
{
    Frame realFrame(myFrame.contentsAsStream());
    send(realFrame);
}

void Node::sendToken()
{
    TokenFrame tokFrame(nodeNum, nodeRight());
    send(tokFrame);
    *errFile << to_string((int) nodeNum) << " sent token to " << (int) nodeRight() << endl; //<<
}

void Node::sendFinished()
{
    FinishedFrame finFrame(nodeNum);
    send(finFrame);
    *errFile << to_string((int) nodeNum) << " sent finished" << endl; //<<
}

bool Node::isForMe(Frame recvdFrame)
{
    return recvdFrame.destAddress() == nodeNum;
}

bool Node::isFromMe(Frame recvdFrame)
{
    return recvdFrame.sourceAddress() == nodeNum;
}

bool Node::isMyToken(Frame recvdFrame)
{
    return isForMe(recvdFrame) and recvdFrame.token();
}

bool Node::amMonitor()
{
    return nodeNum == 1;
}

bool Node::allNodesFinished()
{
    for(unsigned int i=0; i<numNodes; i++) {
        if(!nodesFinished[i])
            return false;
    }
    // Never returned that any node wasn't finished; all are
    return true;
}

unsigned char Node::nodeRight()
{
    return (nodeNum % numNodes) + 1;
}

unsigned char Node::nodeLeft()
{
    return ((numNodes + nodeNum - 2) % numNodes) + 1;
}

string Node::generateOut(Frame myFrame)
{
    string outString = "";
    outString += to_string((int) myFrame.sourceAddress()) + ",";
    outString += to_string((int) myFrame.destAddress()) + ",";
    outString += to_string((int) myFrame.dataSize()) + ",";
    outString += myFrame.data().contentDecoded();
    return outString;
}

string Node::generateFullReadable(Frame myFrame)
{
    string outString = "";
    outString += to_string((int) myFrame.priorityLevel()) + ",";
    outString += myFrame.token() ? "T," : "F,";
    outString += myFrame.monitor() ? "T," : "F,";
    outString += to_string((int) myFrame.reservationLevel()) + ";";
    outString += myFrame.token() ? "T;" : "F;";
    outString += to_string((int) myFrame.destAddress()) + ";";
    outString += to_string((int) myFrame.sourceAddress()) + ";";
    outString += to_string((int) myFrame.dataSize()) + ";";
    outString += myFrame.data().contentDecoded() + ";";
    outString += myFrame.finished() ? "T," : "F,";
    outString += to_string((int) myFrame.status()) + ".";
    return outString;
}

void Node::writeOut(Frame myFrame)
{
    *outFile << generateOut(myFrame) << endl;
}

bool Node::willRejectFrame()
{
    return (rand() % 100 + 1) <= 20;
}

bool Node::wasRejected(Frame myFrame)
{
    return myFrame.status() == 3;
}

bool Node::willForwardAckNak()
{
    return (rand() % 100 + 1) <= 2;
}

bool Node::willLoseToken()
{
    return (rand() % 100 + 1) <= 5;
}

bool Node::willGarbleFrame()
{
    return (rand() % 100 + 1) == 1;
}

Node::~Node()
{
    delete rightNode;
    delete leftNode;
    delete myServerForLeftNode;
    delete inFile;
    delete outFile;
    delete errFile;
    free(nodesFinished);
}

void Node::transmit()
{
    *errFile << to_string((int) nodeNum) << " transmitting!" << endl; //<<
    int curTHT = 0;
    while(haveToken) {
        if(transmitQueue.empty()) {
            string currentLine;
            if(inFile->is_open() and getline(*inFile, currentLine)) {
                // Have more to transmit
                Frame newFrame(currentLine, nodeNum);
                push(newFrame);
            } else {
                // Done transmitting!
                sendFinished();
                sendToken();
                return;
            }
        }

        while(!transmitQueue.empty()) {
            if(curTHT < maxTHT) {
                // Still under token holding time; transmit
                Frame txFrame = pop();
                *errFile << to_string((int) nodeNum) << " popped frame from queue" << endl; //<<
                curTHT += txFrame.dataSize();
                send(txFrame);
            } else {
                // Past token holding time; pass the token
                haveToken = false;
                sendToken();
                break;
            }   
        }
    }
}

void Node::listen()
{
    *errFile << to_string((int) nodeNum) << " listening!" << endl; //<<
    while(!haveToken) {
        bool drainFrame = false;
        bool incomplete = true;
        Frame recvdFrame;
        while(incomplete) {
            // Wait 
            recvdFrame = receiveNextFrame();
            incomplete = recvdFrame.isIncomplete();
            if(incomplete)
                *errFile << to_string((int) nodeNum) << " has incomplete frame." << endl; //<<
        }
        *errFile << to_string((int) nodeNum) << " received frame " << generateFullReadable(recvdFrame) << endl; //<<
        if(isMyToken(recvdFrame)) {
            haveToken = true;
            return;
        }
        if(recvdFrame.finished()) // Neat! A node is finished transmitting.
            nodesFinished[recvdFrame.sourceAddress() - 1] = true;
        if(amMonitor()) {
            // Detect lost tokens <<--TODO
            // Detect garbled frame <<--TODO
            // Detect orphaned frames
            if(recvdFrame.monitor())
                drainFrame = true;
            else
                recvdFrame.setMonitor();
        }

        if(isForMe(recvdFrame)) {
            if(recvdFrame.status() == 0) { // Check that not already seen
                if(willRejectFrame()) {
                    recvdFrame.setStatus(3); // 3 is seen and rejected
                } else {
                    recvdFrame.setStatus(2); // 2 is seen and accepted
                    writeOut(recvdFrame);
                }
            }
        }

        if(isFromMe(recvdFrame)) {
            *errFile << to_string((int) nodeNum) << " flushing own frame " << generateFullReadable(recvdFrame) << endl; //<< was here
            if(recvdFrame.finished())
                drainFrame = true;
            else
                drainFrame = not willForwardAckNak();

            if(wasRejected(recvdFrame)) {
                drainFrame = true;
                recvdFrame.setMonitor(false);
                recvdFrame.setStatus(0); // Reset to 0 to reinitialize
                push(recvdFrame);
            }
        }

        if(!drainFrame)
            send(recvdFrame);

    }
}

void Node::run()
{
    *errFile << to_string((int) nodeNum) << " running!" << endl; //<<
    if(amMonitor()) {
        *errFile << to_string((int) nodeNum) << " sending first token!" << endl; //<<
        sendToken();
    }
    while(!allNodesFinished()) {
        listen();
        transmit();
    }
}

Node::Node(unsigned char nodeNum, unsigned int numNodes, string servAddr, unsigned short portStart) throw(NodeException)
{
    bool connected = false;
    bool listening = false;
    int attempts = 0;

    this->nodeNum = nodeNum;
    this->numNodes = numNodes;
    this->servAddr = servAddr;
    nodesFinished = new bool[numNodes];
    unsigned short myPort = portStart + nodeNum - 1;
    unsigned short rightPort = portStart + nodeRight() - 1;
    // rightNodePort = portStart + nodeLeft() - 1;
    string inFileName = "input-file-" + to_string((int) nodeNum);
    string outFileName = "output-file-" + to_string((int) nodeNum);
    string errFileName = "error-file-" + to_string((int) nodeNum); //<<
    inFile = new ifstream(inFileName);
    outFile = new ofstream(outFileName);
    errFile = new ofstream(errFileName); //<<
    haveToken = false;
    leftovers = BitStream();

    stringstream exceptionHistory;

    while((not connected or not listening) and attempts < 5) {
        try {
            if(not listening) {
                myServerForLeftNode = new TCPServerSocket(myPort);
                // leftNode = new UDPSocket(myPort);
                listening = true;
            }
            rightNode = new TCPSocket(servAddr, rightPort);
            // rightNode = new UDPSocket;
            connected = true;
            leftNode = myServerForLeftNode->accept();

            if(amMonitor()) {
                TokenFrame tokFrame(nodeNum, nodeRight());
                send(tokFrame);
            }
        } catch(SocketException &e) {
            string s(e.what());
            exceptionHistory << s << endl;
            sleep(attempts++); // Maybe waiting a little longer will help?
        }
    }

    if(attempts >= 5)
        throw NodeException("Node couldn't create connections! History: " + exceptionHistory.str());
}
