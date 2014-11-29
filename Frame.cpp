#include "Frame.h"
#include "PracticalSocket.h"
#include "BitStream.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <algorithm>
#include <sstream>

using namespace std;

// FrameByte

BitStream FrameByte::contentsAsStream()
{
    return BitStream(contents);
}

FrameByte::FrameByte()
{
    
}

FrameByte::FrameByte(byte contents)
{
    this->contents = contents;
}

FrameByte::FrameByte(BitStream contents)
{
    this->contents = contents.contentByte();
}

// AccessControl

bool AccessControl::monitor()
{
    return (contents & 0x08) != 0;
}

bool AccessControl::token()
{
    return (contents & 0x10) == 0;
}

unsigned char AccessControl::reservationLevel()
{
    return contents & 0x07;
}

unsigned char AccessControl::priorityLevel()
{
    return (contents & 0xE0) >> 5;
}

void AccessControl::setMonitor(bool monitor)
{
    if (monitor) {
        contents |= 0x08;
    } else {
        contents &= 0xF7;
    }
}

AccessControl::AccessControl(byte contents)
{
    this->contents = contents;
}

AccessControl::AccessControl(bool token, bool monitor, unsigned char priorityLevel, unsigned char reservationLevel) {
    contents = 0;
    contents |= token ? 0x00 : 0x10;
    contents |= monitor ? 0x08 : 0x00;
    contents |= reservationLevel & 0x07;
    contents |= (priorityLevel & 0xE0) << 5;
}

AccessControl::AccessControl(BitStream contents)
{
    this->contents = contents.contentByte();
}

// FrameControl

bool FrameControl::token()
{
    return contents == 0;
}

FrameControl::FrameControl(byte contents)
{
    this->contents = contents;
}

FrameControl::FrameControl(bool token)
{
    contents = token ? 0x00 : 0x01;
}

FrameControl::FrameControl(BitStream contents)
{
    this->contents = contents.contentByte();
}

// FrameAddress

unsigned char FrameAddress::address()
{
    return (unsigned char) contents;
}

FrameAddress::FrameAddress(BitStream contents)
{
    this->contents = contents.contentByte();
}

FrameAddress::FrameAddress(byte contents)
{
    this->contents = contents;
}

FrameAddress::FrameAddress(unsigned char address)
{
    contents = (byte) address;
}

// FrameData

FrameData::~FrameData()
{
    free(contents);
    mySize = 0;
}

BitStream FrameData::sizeAndDataAsStream()
{
    return this->sizeAsStream() + this->dataAsStream();
}

BitStream FrameData::dataAsStream()
{
    return BitStream(mySize, contents);
}

BitStream FrameData::sizeAsStream()
{
    return BitStream((byte) mySize);
}

unsigned char FrameData::size()
{
    return this->mySize;
}

FrameData::FrameData()
{
    mySize = 0;
    contents = NULL;
}

FrameData::FrameData(BitStream contents)
{
    mySize = contents.numBytes();
    this->contents = (byte *) calloc(mySize, sizeof(byte));
    for(unsigned int i=0; i<mySize; i++) {
        (this->contents)[i] = contents.contentByte(i);
    }
}

FrameData::FrameData(unsigned char size, byte *contents)
{
    mySize = size;
    this->contents = (byte *) memcpy(calloc(size, sizeof(byte)), contents, size * sizeof(char));
}

FrameData::FrameData(unsigned char size, BitStream contents)
{
    mySize = size;
    this->contents = (byte *) calloc(size, sizeof(byte));
    for(unsigned int i=0; i<size; i++) {
        (this->contents)[i] = contents.contentByte(i);
    }
}

// FrameStatus

unsigned char FrameStatus::status()
{
    return (unsigned char) (contents & 0x03);
}

bool FrameStatus::finished()
{
    return (contents & 0x80) > 0;
}

void FrameStatus::setStatus(unsigned char status)
{
    contents |= (byte) ((byte) status & 0x03);
}

FrameStatus::FrameStatus(BitStream contents)
{
    this->contents = contents.contentByte();
}

FrameStatus::FrameStatus(byte contents)
{
    this->contents = contents;
}

FrameStatus::FrameStatus(unsigned char status, bool finishedFlag)
{
    contents = (byte) ((byte) status & 0x03) + ((byte) finishedFlag ? 0x80 : 0x00);
}

// Frame

BitStream Frame::contentsAsStream()
{
    return AC->contentsAsStream() + FC->contentsAsStream() + DA->contentsAsStream() + SA->contentsAsStream() + DATA->sizeAndDataAsStream() + FS->contentsAsStream();
}

BitStream Frame::leftovers()
{
    return *myLeftovers;
}

bool Frame::isIncomplete()
{
    return incomplete;
}

int Frame::priorityLevel()
{
    return AC->priorityLevel();
}

bool Frame::token()
{
    return AC->token() or FC->token();
}

bool Frame::monitor()
{
    return AC->monitor();
}

int Frame::reservationLevel()
{
    return AC->reservationLevel();
}

unsigned char Frame::sourceAddress()
{
    return SA->address();
}

unsigned char Frame::destAddress()
{
    return DA->address();
}

unsigned char Frame::dataSize()
{
    return DATA->size();
}

BitStream Frame::data()
{
    return DATA->dataAsStream();
}

unsigned char Frame::status()
{
    return FS->status();
}

bool Frame::finished()
{
    return FS->finished();
}

void Frame::setMonitor(bool monitor)
{
    AC->setMonitor(monitor);
}

void Frame::setStatus(unsigned char status)
{
    FS->setStatus(status);
}

Frame::Frame()
{
    incomplete = true;
    AC = (AccessControl *) NULL;
    FC = (FrameControl *) NULL;
    DA = (FrameAddress *) NULL;
    SA = (FrameAddress *) NULL;
    DATA = (FrameData *) NULL;
    FS = (FrameStatus *) NULL;
    myLeftovers = new BitStream();
}

Frame::Frame(BitStream contents)
{
    if((contents.numBytes() < (unsigned int)6) || (contents.numBytes() < ((unsigned int) 6 + contents.contentByte(4)))) {
        myLeftovers = new BitStream(contents);
        incomplete = true;
    } else {
        AC = new AccessControl(contents.contentByte(0));
        FC = new FrameControl(contents.contentByte(1));
        DA = new FrameAddress(contents.contentByte(2));
        SA = new FrameAddress(contents.contentByte(3));
        unsigned char dataSize = (unsigned char) contents.contentByte(4);
        DATA = new FrameData(dataSize, contents.subStream(5, dataSize));
        FS = new FrameStatus(contents.contentByte(5 + dataSize));
        myLeftovers = new BitStream(contents.subStream(6 + dataSize));
        // cout << "Frame from source " << to_string((int) SA->address()) << " to dest " << to_string((int) DA->address()) << " has contents " << DATA->dataAsStream().contentDecoded() << " and leftovers " << myLeftovers->contentString() << " for total bytes " << to_string(6 + DATA->size()) << " and total stream length " << to_string((int) contents.numBytes()) << " bytes" << endl;
        incomplete = false;
    }
}

Frame::Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, BitStream data, unsigned char status)
{
    AC = new AccessControl(token, monitor, priorityLevel, reservationLevel);
    FC = new FrameControl(token);
    DA = new FrameAddress(sourceAddress);
    SA = new FrameAddress(destAddress);
    DATA = new FrameData(data);
    FS = new FrameStatus(status);
    myLeftovers = new BitStream();
    incomplete = false;
}

Frame::Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, unsigned char dataSize, byte * data, unsigned char status)
{
    AC = new AccessControl(token, monitor, priorityLevel, reservationLevel);
    FC = new FrameControl(token);
    DA = new FrameAddress(sourceAddress);
    SA = new FrameAddress(destAddress);
    DATA = new FrameData(dataSize, data);
    FS = new FrameStatus(status);
    myLeftovers = new BitStream();
    incomplete = false;
}

Frame::Frame(int priorityLevel, bool token, bool monitor, int reservationLevel, unsigned char sourceAddress, unsigned char destAddress, unsigned char dataSize, BitStream data, unsigned char status)
{
    AC = new AccessControl(token, monitor, priorityLevel, reservationLevel);
    FC = new FrameControl(token);
    DA = new FrameAddress(sourceAddress);
    SA = new FrameAddress(destAddress);
    DATA = new FrameData(dataSize, data);
    FS = new FrameStatus(status);
    myLeftovers = new BitStream();
    incomplete = false;
}

Frame::Frame(string inFileLine, unsigned char nodeNum)
{
    int destAddressBuffer;
    int dataSizeBuffer;
    string dataBuffer;

    // Cheat so I can use a stringstream to parse
    replace(inFileLine.begin(), inFileLine.end(), ',', ' ');

    stringstream ss(inFileLine);
    ss >> destAddressBuffer >> dataSizeBuffer >> dataBuffer;

    AC = new AccessControl(false, false, 0, 0);
    FC = new FrameControl(false);
    DA = new FrameAddress((unsigned char) destAddressBuffer);
    SA = new FrameAddress(nodeNum);
    DATA = new FrameData((unsigned char) dataSizeBuffer, (byte *) dataBuffer.c_str());
    FS = new FrameStatus();
    myLeftovers = new BitStream();
    incomplete = false;
}

// TokenFrame

TokenFrame::TokenFrame(BitStream contents) : Frame(contents)
{
    
}

TokenFrame::TokenFrame(unsigned char sourceAddress, unsigned char destAddress)
{
    AC = new AccessControl(true, false, 0, 0);
    FC = new FrameControl(true);
    DA = new FrameAddress(destAddress);
    SA = new FrameAddress(sourceAddress);
    DATA = new FrameData();
    FS = new FrameStatus();
    myLeftovers = new BitStream();
    incomplete = false;
}

// FinishedFrame

FinishedFrame::FinishedFrame(BitStream contents) : Frame(contents)
{
    
}

FinishedFrame::FinishedFrame(unsigned char sourceAddress)
{
    AC = new AccessControl(false, false, 0, 0);
    FC = new FrameControl(false);
    DA = new FrameAddress(sourceAddress);
    SA = new FrameAddress((unsigned char) 0);
    DATA = new FrameData();
    FS = new FrameStatus(0, true);
    myLeftovers = new BitStream();
    incomplete = false;
}
