#include "BitStream.h"

using namespace std;


BitStream::BitStream()
{
    contents = "";
}

BitStream::BitStream(byte myByte)
{
    string bin = "01"; // The positions in bin correspond to the characters
    int bitsRemaining = 8; // Use for 0-padding
    string outstring;
    do {
        outstring = bin[myByte % 2] + outstring;
        myByte /= 2;
    } while (--bitsRemaining);
    contents = outstring;
}

BitStream::BitStream(unsigned int size, byte *bytes)
{
    contents = "";
    for(unsigned int i=0; i<size; i++) {
        BitStream nextSegment(bytes[i]);
        contents += nextSegment.contents;
    }
}

BitStream::BitStream(unsigned int size, char *buffer)
{
    string newContents(buffer, size);
    contents = newContents;
}

BitStream::BitStream(const BitStream& myStream)
{
    contents = myStream.contents.substr(0);
}

BitStream::BitStream(string bitStreamContentString)
{
    contents = bitStreamContentString.substr(0);
}

BitStream& BitStream::operator+=(const BitStream& rhs)
{
    contents = contents + rhs.contents;
    return *this;
}

BitStream& BitStream::operator=(const BitStream& rhs)
{
    BitStream tmp(rhs);
    this->swap(tmp);
    return *this;
}

string BitStream::contentString()
{
    return contents;
}

string BitStream::contentDecoded()
{
    size_t stringSize = contents.length()/8;
    byte byteArr[stringSize];
    string *outString;
    for(unsigned int i=0; i<stringSize; i++) {
        byteArr[i] = this->contentByte(i);
    }
    outString = new string((char *) byteArr, stringSize);
    return *outString;
}

byte BitStream::contentByte(unsigned int byteNum)
{
    byte retbyte = 0;
    string bin = "01";
    for(unsigned int i=0; i<8; i++) {
        retbyte <<= 1;
        retbyte += bin.find(contents[byteNum*8 + i]);
    }
    return retbyte;
}

size_t BitStream::numBytes()
{
    return contents.length()/8;
}

BitStream BitStream::subStream(unsigned int byteStart, size_t numBytes)
{
    size_t myNumBytes = numBytes;
    BitStream newStream;
    if (numBytes == (size_t) -1) {
        myNumBytes = this->numBytes() - byteStart;
        if(myNumBytes == 0) {
            return newStream;
        } else {
            // cout << "Rest of stream? (Stream has " << to_string((int) this->numBytes()) << " bytes, and byteStart is " << to_string(byteStart) << ", so we need to return " << to_string((int) myNumBytes) << " more bytes)" << endl;
        }
    }
    while (myNumBytes + byteStart > this->numBytes())
        myNumBytes--;
    for(unsigned int i=0; i < myNumBytes; i++) {
        BitStream nextSegment(contentByte(byteStart + i));
        newStream += nextSegment;
    }
    return newStream;
}

void BitStream::swap(BitStream& rhs)
{
    using std::swap;
    swap(this->contents, rhs.contents);
}

BitStream operator+(BitStream lhs, const BitStream &rhs)
{
    return lhs += rhs;
}

ostream& operator<<(ostream&out, BitStream &bs)
{
    return out << bs.contentString();
}
