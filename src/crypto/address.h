#ifndef DOGE_ADDRESS_H
#define DOGE_ADDRESS_H

#include <string>
#include <vector>
#include <cstdint>

namespace doge {

// Generate Dogecoin address from public key
// mainnet: version 0x1e (produces 'D' prefix)
// testnet: version 0x71 (produces 'n' prefix)
std::string public_key_to_address(const std::vector<uint8_t>& public_key,
                                   bool mainnet = true);

// Validate Dogecoin address format
bool validate_address(const std::string& address, bool mainnet = true);

// Get address from WIF private key
std::string wif_to_address(const std::string& wif);

} // namespace doge

#endif // DOGE_ADDRESS_H
