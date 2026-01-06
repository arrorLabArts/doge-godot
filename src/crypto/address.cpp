#include "address.h"
#include "base58.h"
#include "keypair.h"
#include "../utils/hash.h"
#include <cstring>

namespace doge {

std::string public_key_to_address(const std::vector<uint8_t>& public_key,
                                   bool mainnet) {
    if (public_key.size() != 33 && public_key.size() != 65) {
        return ""; // Invalid public key size
    }

    // Version byte: 0x1e for mainnet, 0x71 for testnet
    uint8_t version = mainnet ? 0x1e : 0x71;

    // Calculate hash160 (RIPEMD160(SHA256(public_key)))
    uint8_t pubkey_hash[20];
    hash160(public_key.data(), public_key.size(), pubkey_hash);

    // Build payload: version + pubkey_hash
    std::vector<uint8_t> payload;
    payload.push_back(version);
    payload.insert(payload.end(), pubkey_hash, pubkey_hash + 20);

    // Base58Check encode
    return base58check_encode(payload);
}

bool validate_address(const std::string& address, bool mainnet) {
    std::vector<uint8_t> payload;
    if (!base58check_decode(address, payload)) {
        return false;
    }

    // Check payload length (21 bytes: 1 version + 20 hash)
    if (payload.size() != 21) {
        return false;
    }

    // Check version byte
    uint8_t version = payload[0];
    uint8_t expected_version = mainnet ? 0x1e : 0x71;

    return version == expected_version;
}

std::string wif_to_address(const std::string& wif) {
    std::vector<uint8_t> private_key;
    bool compressed;
    bool mainnet;

    if (!wif_to_private_key(wif, private_key, compressed, mainnet)) {
        return "";
    }

    std::vector<uint8_t> public_key;
    if (!derive_public_key(private_key, public_key, compressed)) {
        return "";
    }

    return public_key_to_address(public_key, mainnet);
}

} // namespace doge
