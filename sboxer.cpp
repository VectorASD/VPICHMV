#include "sboxer.h"

Sboxer::Sboxer() {
    std::set<byte> nums;
    for (int i = 64; i < 256; i++) nums.insert(byte(i));
    int size = int(nums.size()), pos = 0;
    do {
        int i = rand() % size;
        nums.begin();
        auto it = std::next(nums.begin(), i);
        sbox[pos] = *it;
        rsbox[*it] = byte(pos++);
        nums.erase(it);
        size = int(nums.size());
    } while (size);
}

byte Sboxer::id2num(uint id, uint shift) {
    uint num = 0, pos = 0;
    for (uint i = shift; i < 24; i += 3) num |= (id >> i & 1) << (pos++);
    return sbox[num];
}

uint Sboxer::num2id(byte num, uint shift) {
    num = rsbox[num];
    uint res = 0, pos = 0;
    for (uint i = shift; i < 24; i += 3) res |= uint((num >> (pos++) & 1) << i);
    return res;
}





UID Sboxers::id2uid(uint id) {
    return { id, R.id2num(id, 0), G.id2num(id, 1), B.id2num(id, 2) };
}

UID Sboxers::color2uid(byte r, byte g, byte b) {
    if (r < 64 || b < 64 || g < 64) return { 0, r, g, b };

    uint id = R.num2id(r, 0) | G.num2id(g, 1) | B.num2id(b, 2);
    return { id, r, g, b };
}
