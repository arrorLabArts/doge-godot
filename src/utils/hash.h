#ifndef DOGE_HASH_H
#define DOGE_HASH_H

#include <cstdint>
#include <vector>

namespace doge {

// SHA256 hash function
void sha256(const uint8_t* data, size_t len, uint8_t* hash);
void sha256(const std::vector<uint8_t>& data, uint8_t* hash);

// Double SHA256 (used for message signing)
void sha256_double(const uint8_t* data, size_t len, uint8_t* hash);
void sha256_double(const std::vector<uint8_t>& data, uint8_t* hash);

// RIPEMD160 hash function (used for address generation)
void ripemd160(const uint8_t* data, size_t len, uint8_t* hash);
void ripemd160(const std::vector<uint8_t>& data, uint8_t* hash);

// Hash160: RIPEMD160(SHA256(data)) - used for Bitcoin/Dogecoin addresses
void hash160(const uint8_t* data, size_t len, uint8_t* hash);
void hash160(const std::vector<uint8_t>& data, uint8_t* hash);

} // namespace doge

#endif // DOGE_HASH_H
