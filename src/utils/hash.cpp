#include "hash.h"
#include <cstring>

namespace doge {

// SHA256 implementation (based on public domain code)
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static void sha256_transform(uint32_t* state, const uint8_t* block) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];

    for (int i = 0, j = 0; i < 16; i++, j += 4) {
        m[i] = (block[j] << 24) | (block[j + 1] << 16) | (block[j + 2] << 8) | (block[j + 3]);
    }

    for (int i = 16; i < 64; i++) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = state[0]; b = state[1]; c = state[2]; d = state[3];
    e = state[4]; f = state[5]; g = state[6]; h = state[7];

    for (int i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256(const uint8_t* data, size_t len, uint8_t* hash) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    uint8_t block[64];
    size_t i = 0;
    uint64_t bitlen = len * 8;

    // Process full blocks
    while (i + 64 <= len) {
        sha256_transform(state, data + i);
        i += 64;
    }

    // Handle remaining bytes
    size_t rem = len - i;
    memcpy(block, data + i, rem);
    block[rem++] = 0x80;

    if (rem > 56) {
        memset(block + rem, 0, 64 - rem);
        sha256_transform(state, block);
        rem = 0;
    }

    memset(block + rem, 0, 56 - rem);

    // Append length
    for (int j = 0; j < 8; j++) {
        block[56 + j] = (bitlen >> (56 - j * 8)) & 0xff;
    }

    sha256_transform(state, block);

    // Convert to bytes
    for (int j = 0; j < 8; j++) {
        hash[j * 4] = (state[j] >> 24) & 0xff;
        hash[j * 4 + 1] = (state[j] >> 16) & 0xff;
        hash[j * 4 + 2] = (state[j] >> 8) & 0xff;
        hash[j * 4 + 3] = state[j] & 0xff;
    }
}

void sha256(const std::vector<uint8_t>& data, uint8_t* hash) {
    sha256(data.data(), data.size(), hash);
}

void sha256_double(const uint8_t* data, size_t len, uint8_t* hash) {
    uint8_t temp[32];
    sha256(data, len, temp);
    sha256(temp, 32, hash);
}

void sha256_double(const std::vector<uint8_t>& data, uint8_t* hash) {
    sha256_double(data.data(), data.size(), hash);
}

// RIPEMD160 implementation (based on public domain code)
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

#define F(x, y, z) ((x) ^ (y) ^ (z))
#define G(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z) (((x) | ~(y)) ^ (z))
#define I(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z) ((x) ^ ((y) | ~(z)))

#define FF(a, b, c, d, e, x, s) a += F(b, c, d) + x; a = ROL(a, s) + e; c = ROL(c, 10);
#define GG(a, b, c, d, e, x, s) a += G(b, c, d) + x + 0x5a827999; a = ROL(a, s) + e; c = ROL(c, 10);
#define HH(a, b, c, d, e, x, s) a += H(b, c, d) + x + 0x6ed9eba1; a = ROL(a, s) + e; c = ROL(c, 10);
#define II(a, b, c, d, e, x, s) a += I(b, c, d) + x + 0x8f1bbcdc; a = ROL(a, s) + e; c = ROL(c, 10);
#define JJ(a, b, c, d, e, x, s) a += J(b, c, d) + x + 0xa953fd4e; a = ROL(a, s) + e; c = ROL(c, 10);

#define FFF(a, b, c, d, e, x, s) a += F(b, c, d) + x; a = ROL(a, s) + e; c = ROL(c, 10);
#define GGG(a, b, c, d, e, x, s) a += G(b, c, d) + x + 0x7a6d76e9; a = ROL(a, s) + e; c = ROL(c, 10);
#define HHH(a, b, c, d, e, x, s) a += H(b, c, d) + x + 0x6d703ef3; a = ROL(a, s) + e; c = ROL(c, 10);
#define III(a, b, c, d, e, x, s) a += I(b, c, d) + x + 0x5c4dd124; a = ROL(a, s) + e; c = ROL(c, 10);
#define JJJ(a, b, c, d, e, x, s) a += J(b, c, d) + x + 0x50a28be6; a = ROL(a, s) + e; c = ROL(c, 10);

static void ripemd160_transform(uint32_t* state, const uint8_t* block) {
    uint32_t al, bl, cl, dl, el, ar, br, cr, dr, er;
    uint32_t x[16];

    for (int i = 0; i < 16; i++) {
        x[i] = block[i * 4] | (block[i * 4 + 1] << 8) |
               (block[i * 4 + 2] << 16) | (block[i * 4 + 3] << 24);
    }

    al = ar = state[0];
    bl = br = state[1];
    cl = cr = state[2];
    dl = dr = state[3];
    el = er = state[4];

    // Left rounds
    FF(al, bl, cl, dl, el, x[ 0], 11); FF(el, al, bl, cl, dl, x[ 1], 14); FF(dl, el, al, bl, cl, x[ 2], 15);
    FF(cl, dl, el, al, bl, x[ 3], 12); FF(bl, cl, dl, el, al, x[ 4],  5); FF(al, bl, cl, dl, el, x[ 5],  8);
    FF(el, al, bl, cl, dl, x[ 6],  7); FF(dl, el, al, bl, cl, x[ 7],  9); FF(cl, dl, el, al, bl, x[ 8], 11);
    FF(bl, cl, dl, el, al, x[ 9], 13); FF(al, bl, cl, dl, el, x[10], 14); FF(el, al, bl, cl, dl, x[11], 15);
    FF(dl, el, al, bl, cl, x[12],  6); FF(cl, dl, el, al, bl, x[13],  7); FF(bl, cl, dl, el, al, x[14],  9);
    FF(al, bl, cl, dl, el, x[15],  8);

    GG(el, al, bl, cl, dl, x[ 7],  7); GG(dl, el, al, bl, cl, x[ 4],  6); GG(cl, dl, el, al, bl, x[13],  8);
    GG(bl, cl, dl, el, al, x[ 1], 13); GG(al, bl, cl, dl, el, x[10], 11); GG(el, al, bl, cl, dl, x[ 6],  9);
    GG(dl, el, al, bl, cl, x[15],  7); GG(cl, dl, el, al, bl, x[ 3], 15); GG(bl, cl, dl, el, al, x[12],  7);
    GG(al, bl, cl, dl, el, x[ 0], 12); GG(el, al, bl, cl, dl, x[ 9], 15); GG(dl, el, al, bl, cl, x[ 5],  9);
    GG(cl, dl, el, al, bl, x[ 2], 11); GG(bl, cl, dl, el, al, x[14],  7); GG(al, bl, cl, dl, el, x[11], 13);
    GG(el, al, bl, cl, dl, x[ 8], 12);

    HH(dl, el, al, bl, cl, x[ 3], 11); HH(cl, dl, el, al, bl, x[10], 13); HH(bl, cl, dl, el, al, x[14],  6);
    HH(al, bl, cl, dl, el, x[ 4],  7); HH(el, al, bl, cl, dl, x[ 9], 14); HH(dl, el, al, bl, cl, x[15],  9);
    HH(cl, dl, el, al, bl, x[ 8], 13); HH(bl, cl, dl, el, al, x[ 1], 15); HH(al, bl, cl, dl, el, x[ 2], 14);
    HH(el, al, bl, cl, dl, x[ 7],  8); HH(dl, el, al, bl, cl, x[ 0], 13); HH(cl, dl, el, al, bl, x[ 6],  6);
    HH(bl, cl, dl, el, al, x[13],  5); HH(al, bl, cl, dl, el, x[11], 12); HH(el, al, bl, cl, dl, x[ 5],  7);
    HH(dl, el, al, bl, cl, x[12],  5);

    II(cl, dl, el, al, bl, x[ 1], 11); II(bl, cl, dl, el, al, x[ 9], 12); II(al, bl, cl, dl, el, x[11], 14);
    II(el, al, bl, cl, dl, x[10], 15); II(dl, el, al, bl, cl, x[ 0], 14); II(cl, dl, el, al, bl, x[ 8], 15);
    II(bl, cl, dl, el, al, x[12],  9); II(al, bl, cl, dl, el, x[ 4],  8); II(el, al, bl, cl, dl, x[13],  9);
    II(dl, el, al, bl, cl, x[ 3], 14); II(cl, dl, el, al, bl, x[ 7],  5); II(bl, cl, dl, el, al, x[15],  6);
    II(al, bl, cl, dl, el, x[14],  8); II(el, al, bl, cl, dl, x[ 5],  6); II(dl, el, al, bl, cl, x[ 6],  5);
    II(cl, dl, el, al, bl, x[ 2], 12);

    JJ(bl, cl, dl, el, al, x[ 4],  9); JJ(al, bl, cl, dl, el, x[ 0], 15); JJ(el, al, bl, cl, dl, x[ 5],  5);
    JJ(dl, el, al, bl, cl, x[ 9], 11); JJ(cl, dl, el, al, bl, x[ 7],  6); JJ(bl, cl, dl, el, al, x[12],  8);
    JJ(al, bl, cl, dl, el, x[ 2], 13); JJ(el, al, bl, cl, dl, x[10], 12); JJ(dl, el, al, bl, cl, x[14],  5);
    JJ(cl, dl, el, al, bl, x[ 1], 12); JJ(bl, cl, dl, el, al, x[ 3], 13); JJ(al, bl, cl, dl, el, x[ 8], 14);
    JJ(el, al, bl, cl, dl, x[11], 11); JJ(dl, el, al, bl, cl, x[ 6],  8); JJ(cl, dl, el, al, bl, x[15],  5);
    JJ(bl, cl, dl, el, al, x[13],  6);

    // Right rounds
    JJJ(ar, br, cr, dr, er, x[ 5],  8); JJJ(er, ar, br, cr, dr, x[14],  9); JJJ(dr, er, ar, br, cr, x[ 7],  9);
    JJJ(cr, dr, er, ar, br, x[ 0], 11); JJJ(br, cr, dr, er, ar, x[ 9], 13); JJJ(ar, br, cr, dr, er, x[ 2], 15);
    JJJ(er, ar, br, cr, dr, x[11], 15); JJJ(dr, er, ar, br, cr, x[ 4],  5); JJJ(cr, dr, er, ar, br, x[13],  7);
    JJJ(br, cr, dr, er, ar, x[ 6],  7); JJJ(ar, br, cr, dr, er, x[15],  8); JJJ(er, ar, br, cr, dr, x[ 8], 11);
    JJJ(dr, er, ar, br, cr, x[ 1], 14); JJJ(cr, dr, er, ar, br, x[10], 14); JJJ(br, cr, dr, er, ar, x[ 3], 12);
    JJJ(ar, br, cr, dr, er, x[12],  6);

    III(er, ar, br, cr, dr, x[ 6],  9); III(dr, er, ar, br, cr, x[11], 13); III(cr, dr, er, ar, br, x[ 3], 15);
    III(br, cr, dr, er, ar, x[ 7],  7); III(ar, br, cr, dr, er, x[ 0], 12); III(er, ar, br, cr, dr, x[13],  8);
    III(dr, er, ar, br, cr, x[ 5],  9); III(cr, dr, er, ar, br, x[10], 11); III(br, cr, dr, er, ar, x[14],  7);
    III(ar, br, cr, dr, er, x[15],  7); III(er, ar, br, cr, dr, x[ 8], 12); III(dr, er, ar, br, cr, x[12],  7);
    III(cr, dr, er, ar, br, x[ 4],  6); III(br, cr, dr, er, ar, x[ 9], 15); III(ar, br, cr, dr, er, x[ 1], 13);
    III(er, ar, br, cr, dr, x[ 2], 11);

    HHH(dr, er, ar, br, cr, x[15],  9); HHH(cr, dr, er, ar, br, x[ 5],  7); HHH(br, cr, dr, er, ar, x[ 1], 15);
    HHH(ar, br, cr, dr, er, x[ 3], 11); HHH(er, ar, br, cr, dr, x[ 7],  8); HHH(dr, er, ar, br, cr, x[14],  6);
    HHH(cr, dr, er, ar, br, x[ 6],  6); HHH(br, cr, dr, er, ar, x[ 9], 14); HHH(ar, br, cr, dr, er, x[11], 12);
    HHH(er, ar, br, cr, dr, x[ 8], 13); HHH(dr, er, ar, br, cr, x[12],  5); HHH(cr, dr, er, ar, br, x[ 2], 14);
    HHH(br, cr, dr, er, ar, x[10], 13); HHH(ar, br, cr, dr, er, x[ 0], 13); HHH(er, ar, br, cr, dr, x[ 4],  7);
    HHH(dr, er, ar, br, cr, x[13],  5);

    GGG(cr, dr, er, ar, br, x[ 8], 15); GGG(br, cr, dr, er, ar, x[ 6],  5); GGG(ar, br, cr, dr, er, x[ 4],  8);
    GGG(er, ar, br, cr, dr, x[ 1], 11); GGG(dr, er, ar, br, cr, x[ 3], 14); GGG(cr, dr, er, ar, br, x[11], 14);
    GGG(br, cr, dr, er, ar, x[15],  6); GGG(ar, br, cr, dr, er, x[ 0], 14); GGG(er, ar, br, cr, dr, x[ 5],  6);
    GGG(dr, er, ar, br, cr, x[12],  9); GGG(cr, dr, er, ar, br, x[ 2], 12); GGG(br, cr, dr, er, ar, x[13],  9);
    GGG(ar, br, cr, dr, er, x[ 9], 12); GGG(er, ar, br, cr, dr, x[ 7],  5); GGG(dr, er, ar, br, cr, x[10], 15);
    GGG(cr, dr, er, ar, br, x[14],  8);

    FFF(br, cr, dr, er, ar, x[12],  8); FFF(ar, br, cr, dr, er, x[15],  5); FFF(er, ar, br, cr, dr, x[10], 12);
    FFF(dr, er, ar, br, cr, x[ 4],  9); FFF(cr, dr, er, ar, br, x[ 1], 12); FFF(br, cr, dr, er, ar, x[ 5],  5);
    FFF(ar, br, cr, dr, er, x[ 8], 14); FFF(er, ar, br, cr, dr, x[ 7],  6); FFF(dr, er, ar, br, cr, x[ 6],  8);
    FFF(cr, dr, er, ar, br, x[ 2], 13); FFF(br, cr, dr, er, ar, x[13],  6); FFF(ar, br, cr, dr, er, x[14],  5);
    FFF(er, ar, br, cr, dr, x[ 0], 15); FFF(dr, er, ar, br, cr, x[ 3], 13); FFF(cr, dr, er, ar, br, x[ 9], 11);
    FFF(br, cr, dr, er, ar, x[11], 11);

    uint32_t t = state[1] + cl + dr;
    state[1] = state[2] + dl + er;
    state[2] = state[3] + el + ar;
    state[3] = state[4] + al + br;
    state[4] = state[0] + bl + cr;
    state[0] = t;
}

void ripemd160(const uint8_t* data, size_t len, uint8_t* hash) {
    uint32_t state[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};

    uint8_t block[64];
    size_t i = 0;
    uint64_t bitlen = len * 8;

    // Process full blocks
    while (i + 64 <= len) {
        ripemd160_transform(state, data + i);
        i += 64;
    }

    // Handle remaining bytes
    size_t rem = len - i;
    memcpy(block, data + i, rem);
    block[rem++] = 0x80;

    if (rem > 56) {
        memset(block + rem, 0, 64 - rem);
        ripemd160_transform(state, block);
        rem = 0;
    }

    memset(block + rem, 0, 56 - rem);

    // Append length (little-endian)
    for (int j = 0; j < 8; j++) {
        block[56 + j] = (bitlen >> (j * 8)) & 0xff;
    }

    ripemd160_transform(state, block);

    // Convert to bytes (little-endian)
    for (int j = 0; j < 5; j++) {
        hash[j * 4] = state[j] & 0xff;
        hash[j * 4 + 1] = (state[j] >> 8) & 0xff;
        hash[j * 4 + 2] = (state[j] >> 16) & 0xff;
        hash[j * 4 + 3] = (state[j] >> 24) & 0xff;
    }
}

void ripemd160(const std::vector<uint8_t>& data, uint8_t* hash) {
    ripemd160(data.data(), data.size(), hash);
}

void hash160(const uint8_t* data, size_t len, uint8_t* hash) {
    uint8_t sha_hash[32];
    sha256(data, len, sha_hash);
    ripemd160(sha_hash, 32, hash);
}

void hash160(const std::vector<uint8_t>& data, uint8_t* hash) {
    hash160(data.data(), data.size(), hash);
}

} // namespace doge
