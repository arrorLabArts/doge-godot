#ifndef DOGE_KEYPAIR_H
#define DOGE_KEYPAIR_H

#include <string>
#include <vector>
#include <cstdint>

namespace doge {

// Generate a random private key (32 bytes)
bool generate_private_key(std::vector<uint8_t>& private_key);

// Derive public key from private key (compressed or uncompressed)
bool derive_public_key(const std::vector<uint8_t>& private_key,
                       std::vector<uint8_t>& public_key,
                       bool compressed = true);

// Convert private key to WIF (Wallet Import Format)
// mainnet: version 0x9e, testnet: version 0xf1
std::string private_key_to_wif(const std::vector<uint8_t>& private_key,
                                bool compressed = true,
                                bool mainnet = true);

// Import private key from WIF
bool wif_to_private_key(const std::string& wif,
                        std::vector<uint8_t>& private_key,
                        bool& compressed,
                        bool& mainnet);

// Hex encoding/decoding helpers
std::string bytes_to_hex(const uint8_t* data, size_t len);
std::string bytes_to_hex(const std::vector<uint8_t>& data);
bool hex_to_bytes(const std::string& hex, std::vector<uint8_t>& out);

} // namespace doge

#endif // DOGE_KEYPAIR_H
