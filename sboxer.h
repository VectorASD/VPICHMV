#ifndef SBOXER_H
#define SBOXER_H

#include "stdlib.h"
#include "set"

#include <QDebug>

typedef unsigned char byte;

class Sboxer {
private:
    byte sbox[256 - 64];
    byte rsbox[256];
public:
    Sboxer();
    byte id2num(uint id, uint shift);
    uint num2id(byte num, uint shift);
};

struct UID {
    uint id;
    byte r, g, b;
};

class Sboxers {
private:
    Sboxer R, G, B;
public:
    UID id2uid(uint id);
    UID color2uid(byte r, byte g, byte b);
};

#endif // SBOXER_H
