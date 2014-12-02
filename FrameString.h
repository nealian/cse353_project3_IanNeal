#ifndef __FRAME_STRING_H__
#define __FRAME_STRING_H__

#include <sys/types.h>
#include <string>

using namespace std;

// The access control byte
class AccessControlString {
public:
    bool monitor();
    bool token();
    unsigned char reservationLevel();
    unsigned char priorityLevel();
    void setMonitor(bool monitor = true);

    //Constructors
    AccessControlString(bool token, bool monitor = false, unsigned char priorityLevel = 0, unsigned char reservationLevel = 0);
    AccessControlString(string accessControl);
};


// The frame control byte (tells whether or not it's a token? okay... seems unnecessary)
class FrameControlString {
public:
    bool token();

    //Constructors
    FrameControlString(bool token);
    FrameControlString(string frameControl);
};


// For the destination address and source address bytes
class FrameAddress {
public:
    unsigned char address();

    //Constructors
    FrameAddress(unsigned char address);
    FrameAddress(string frameAddress);
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
    AccessControlString *AC;
    FrameControlString *FC;
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
