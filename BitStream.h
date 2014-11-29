#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <string>
#include <iostream>

using namespace std;

typedef struct byte {
    unsigned char inner;
    byte(unsigned char c): inner(c) {}
    byte() {inner = 0;}
    byte operator%(const int &i) {return inner % i;}
    void operator|=(const int &i) {inner |= i;}
    void operator&=(const int &i) {inner &= i;}
    void operator/=(const int &i) {inner /= i;}
    void operator<<=(const int &i) {inner <<= i;}
    void operator+=(const int &i) {inner += i;}
    operator int() {return inner;}
} byte; // Data is bytes; an unsigned char is (at least) 8 bits

class BitStream {
public:
    BitStream& operator+=(const BitStream& rhs);
    BitStream& operator=(const BitStream& rhs);
    string contentString();
    string contentDecoded();
    byte contentByte(unsigned int byteNum = 0);
    size_t numBytes();
    BitStream subStream(unsigned int byteStart, size_t numBytes = (size_t) -1);
    void swap(BitStream& rhs);

    //Constructors
    BitStream();
    BitStream(byte myByte);
    BitStream(unsigned int size, byte *bytes);
    BitStream(unsigned int size, char *buffer);
    BitStream(const BitStream& myStream);
    BitStream(string bitStreamContentString);

private:
    string contents;
};

BitStream operator+(BitStream lhs, const BitStream& rhs);

ostream& operator<<(ostream&out, const BitStream &bs);


#endif
