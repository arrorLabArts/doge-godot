#ifndef DOGE_BASE58_H
#define DOGE_BASE58_H

#include <string>
#include <vector>
#include <cstdint>

namespace doge {

// Base58 encoding/decoding (without checksum)
std::string base58_encode(const uint8_t* data, size_t len);
std::string base58_encode(const std::vector<uint8_t>& data);

bool base58_decode(const std::string& str, std::vector<uint8_t>& out);

// Base58Check encoding/decoding (with checksum)
// Checksum is first 4 bytes of SHA256(SHA256(payload))
std::string base58check_encode(const uint8_t* data, size_t len);
std::string base58check_encode(const std::vector<uint8_t>& data);

bool base58check_decode(const std::string& str, std::vector<uint8_t>& out);

} // namespace doge

#endif // DOGE_BASE58_H
