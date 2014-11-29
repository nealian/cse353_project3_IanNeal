#ifndef __FRAME_H__
#define __FRAME_H__

#include <sys/types.h>
#include <string>
#include "BitStream.h"

using namespace std;

// The basic frame byte
class FrameByte {
public:
    BitStream contentsAsStream(); // Will output stream of 1's and 0's

    //Constructors
    FrameByte();
    FrameByte(byte contents);
    FrameByte(BitStream contents);

protected:
    byte contents;
};

// The access control byte
class AccessControl : public FrameByte {
public:
    bool monitor();
    bool token();
    unsigned char reservationLevel();
    unsigned char priorityLevel();
    void setMonitor(bool monitor = true);

    //Constructors
    AccessControl(byte contents);
    AccessControl(bool token, bool monitor = false, unsigned char priorityLevel = 0, unsigned char reservationLevel = 0);
    AccessControl(BitStream contents);
};


// The frame control byte (tells whether or not it's a token? okay... seems unnecessary)
class FrameControl : public FrameByte {
public:
    bool token();

    //Constructors
    FrameControl(byte contents);
    FrameControl(bool token);
    FrameControl(BitStream contents);
};


// For the destination address and source address bytes
class FrameAddress : public FrameByte {
public:
    unsigned char address();

    //Constructors
    FrameAddress(BitStream contents);
    FrameAddress(byte contents);
    FrameAddress(unsigned char address);
};


// The actual data!
class FrameData {
public:
    //Destructor! since it has a pointer.
    ~FrameData();

    BitStream sizeAndDataAsStream();
    BitStream dataAsStream();
    BitStream sizeAsStream();
    unsigned char size();

    //Constructors
    FrameData();
    FrameData(BitStream contents);
    FrameData(unsigned char size, byte *contents);
    FrameData(unsigned char size, BitStream contents);

protected:
    unsigned char mySize;
    byte *contents;
};


// For the frame status byte; ack/nak and completion synchronization
class FrameStatus : public FrameByte {
public:
    unsigned char status();
    bool finished();
    void setStatus(unsigned char status);

    //Constructors
    FrameStatus(BitStream contents);
    FrameStatus(byte contents);
    FrameStatus(unsigned char status = 0, bool finishedFlag = false);
};


// The full frame!
class Frame {
public:
    BitStream contentsAsStream();
    BitStream leftovers();
    bool isIncomplete();
    int priorityLevel();
    bool token();
    bool monitor();
    int reservationLevel();
    unsigned char sourceAddress();
    unsigned char destAddress();
    unsigned char dataSize();
    BitStream data();
    unsigned char status();
    bool finished();
    void setMonitor(bool monitor = true);
    void setStatus(unsigned char status);

    //Contstructors
    Frame();
    Frame(BitStream contents);
    Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, BitStream data, unsigned char status);
    Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, unsigned char dataSize, byte * data, unsigned char status);
    Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, unsigned char dataSize, BitStream data, unsigned char status);
    Frame(string inFileLine, unsigned char nodeNum);

protected:
    AccessControl *AC;
    FrameControl *FC;
    FrameAddress *DA;
    FrameAddress *SA;
    FrameData *DATA;
    FrameStatus *FS;
    BitStream *myLeftovers; // Basically, if there was anything after the FS byte, it goes here
    bool incomplete; // Does it have enough to fill everything, including data and FS, based on the data size byte?
};


// A token frame is basically a frame with empty data and the token bits set
class TokenFrame : public Frame {
public:
    TokenFrame(BitStream contents);
    TokenFrame(unsigned char sourceAddress, unsigned char destAddress);
};


// A finished frame is for completion synchronization; it uses an extra bit in the FS byte
class FinishedFrame : public Frame {
public:
    FinishedFrame(BitStream contents);
    FinishedFrame(unsigned char sourceAddress);
};

#endif
