#include "base58.h"
#include "../utils/hash.h"
#include <algorithm>
#include <cstring>

namespace doge {

// Base58 alphabet (Bitcoin/Dogecoin standard)
static const char* BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58_encode(const uint8_t* data, size_t len) {
    if (len == 0) return "";

    // Count leading zeros
    size_t leading_zeros = 0;
    while (leading_zeros < len && data[leading_zeros] == 0) {
        leading_zeros++;
    }

    // Allocate enough space for base58 encoding (log(256)/log(58) ~= 1.37)
    size_t size = len * 138 / 100 + 1;
    std::vector<uint8_t> b58(size);

    // Process the bytes
    for (size_t i = 0; i < len; i++) {
        uint32_t carry = data[i];

        // Apply "b58 = b58 * 256 + carry"
        for (size_t j = 0; j < size; j++) {
            carry += 256 * b58[size - j - 1];
            b58[size - j - 1] = carry % 58;
            carry /= 58;
        }
    }

    // Skip leading zeros in b58
    size_t b58_leading_zeros = 0;
    while (b58_leading_zeros < size && b58[b58_leading_zeros] == 0) {
        b58_leading_zeros++;
    }

    // Convert to base58 string
    std::string result;
    result.reserve(leading_zeros + size - b58_leading_zeros);

    // Add '1' for each leading zero byte
    for (size_t i = 0; i < leading_zeros; i++) {
        result += '1';
    }

    // Convert digits to base58 characters
    for (size_t i = b58_leading_zeros; i < size; i++) {
        result += BASE58_ALPHABET[b58[i]];
    }

    return result;
}

std::string base58_encode(const std::vector<uint8_t>& data) {
    return base58_encode(data.data(), data.size());
}

bool base58_decode(const std::string& str, std::vector<uint8_t>& out) {
    if (str.empty()) {
        out.clear();
        return true;
    }

    // Count leading '1's (zeros)
    size_t leading_ones = 0;
    while (leading_ones < str.size() && str[leading_ones] == '1') {
        leading_ones++;
    }

    // Allocate enough space
    size_t size = str.size() * 733 / 1000 + 1;
    std::vector<uint8_t> b256(size);

    // Process the characters
    for (size_t i = 0; i < str.size(); i++) {
        // Find character in alphabet
        const char* p = strchr(BASE58_ALPHABET, str[i]);
        if (!p) {
            return false; // Invalid character
        }

        uint32_t carry = p - BASE58_ALPHABET;

        // Apply "b256 = b256 * 58 + carry"
        for (size_t j = 0; j < size; j++) {
            carry += 58 * b256[size - j - 1];
            b256[size - j - 1] = carry % 256;
            carry /= 256;
        }
    }

    // Skip leading zeros in b256
    size_t b256_leading_zeros = 0;
    while (b256_leading_zeros < size && b256[b256_leading_zeros] == 0) {
        b256_leading_zeros++;
    }

    // Copy result
    out.clear();
    out.reserve(leading_ones + size - b256_leading_zeros);

    // Add zero bytes for leading '1's
    out.insert(out.end(), leading_ones, 0);

    // Copy the rest
    out.insert(out.end(), b256.begin() + b256_leading_zeros, b256.end());

    return true;
}

std::string base58check_encode(const uint8_t* data, size_t len) {
    // Calculate checksum: first 4 bytes of SHA256(SHA256(data))
    uint8_t hash[32];
    sha256_double(data, len, hash);

    // Append checksum to data
    std::vector<uint8_t> with_checksum(data, data + len);
    with_checksum.insert(with_checksum.end(), hash, hash + 4);

    return base58_encode(with_checksum);
}

std::string base58check_encode(const std::vector<uint8_t>& data) {
    return base58check_encode(data.data(), data.size());
}

bool base58check_decode(const std::string& str, std::vector<uint8_t>& out) {
    std::vector<uint8_t> decoded;
    if (!base58_decode(str, decoded)) {
        return false;
    }

    if (decoded.size() < 4) {
        return false; // Too short for checksum
    }

    // Split data and checksum
    size_t payload_len = decoded.size() - 4;
    std::vector<uint8_t> payload(decoded.begin(), decoded.begin() + payload_len);
    std::vector<uint8_t> checksum(decoded.begin() + payload_len, decoded.end());

    // Verify checksum
    uint8_t hash[32];
    sha256_double(payload.data(), payload.size(), hash);

    if (memcmp(hash, checksum.data(), 4) != 0) {
        return false; // Checksum mismatch
    }

    out = payload;
    return true;
}

} // namespace doge
