#include "keypair.h"
#include "base58.h"
#include <secp256k1.h>
#include <cstring>
#include <random>
#include <fstream>

// For secure random number generation
#ifdef __ANDROID__
#include <sys/random.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <Security/Security.h>
#endif

namespace doge {

static secp256k1_context* get_secp256k1_context() {
    static secp256k1_context* ctx = nullptr;
    if (!ctx) {
        ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    }
    return ctx;
}

bool generate_private_key(std::vector<uint8_t>& private_key) {
    private_key.resize(32);

#if defined(__APPLE__) && defined(__MACH__)
    // Use SecRandomCopyBytes on iOS/macOS
    if (SecRandomCopyBytes(kSecRandomDefault, 32, private_key.data()) != errSecSuccess) {
        return false;
    }
#else
    // Use /dev/urandom on Android and Linux (works on all API levels)
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom.read(reinterpret_cast<char*>(private_key.data()), 32)) {
        return false;
    }
#endif

    // Verify the private key is valid
    secp256k1_context* ctx = get_secp256k1_context();
    if (!secp256k1_ec_seckey_verify(ctx, private_key.data())) {
        // Invalid key, try again (very rare)
        return generate_private_key(private_key);
    }

    return true;
}

bool derive_public_key(const std::vector<uint8_t>& private_key,
                       std::vector<uint8_t>& public_key,
                       bool compressed) {
    if (private_key.size() != 32) {
        return false;
    }

    secp256k1_context* ctx = get_secp256k1_context();

    // Verify private key
    if (!secp256k1_ec_seckey_verify(ctx, private_key.data())) {
        return false;
    }

    // Derive public key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, private_key.data())) {
        return false;
    }

    // Serialize public key
    size_t output_len = compressed ? 33 : 65;
    public_key.resize(output_len);

    unsigned int flags = compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;
    if (!secp256k1_ec_pubkey_serialize(ctx, public_key.data(), &output_len, &pubkey, flags)) {
        return false;
    }

    return true;
}

std::string private_key_to_wif(const std::vector<uint8_t>& private_key,
                                bool compressed,
                                bool mainnet) {
    if (private_key.size() != 32) {
        return "";
    }

    // Version byte: 0x9e for mainnet, 0xf1 for testnet
    uint8_t version = mainnet ? 0x9e : 0xf1;

    // Build payload: version + private_key + (0x01 if compressed)
    std::vector<uint8_t> payload;
    payload.push_back(version);
    payload.insert(payload.end(), private_key.begin(), private_key.end());

    if (compressed) {
        payload.push_back(0x01);
    }

    return base58check_encode(payload);
}

bool wif_to_private_key(const std::string& wif,
                        std::vector<uint8_t>& private_key,
                        bool& compressed,
                        bool& mainnet) {
    std::vector<uint8_t> payload;
    if (!base58check_decode(wif, payload)) {
        return false;
    }

    // Check payload length (33 or 34 bytes)
    if (payload.size() != 33 && payload.size() != 34) {
        return false;
    }

    // Extract version byte
    uint8_t version = payload[0];
    if (version == 0x9e) {
        mainnet = true;
    } else if (version == 0xf1) {
        mainnet = false;
    } else {
        return false; // Invalid version
    }

    // Check if compressed
    if (payload.size() == 34) {
        if (payload[33] != 0x01) {
            return false; // Invalid compression flag
        }
        compressed = true;
    } else {
        compressed = false;
    }

    // Extract private key (32 bytes after version)
    private_key.assign(payload.begin() + 1, payload.begin() + 33);

    // Verify the private key is valid
    secp256k1_context* ctx = get_secp256k1_context();
    if (!secp256k1_ec_seckey_verify(ctx, private_key.data())) {
        return false;
    }

    return true;
}

std::string bytes_to_hex(const uint8_t* data, size_t len) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string result;
    result.reserve(len * 2);

    for (size_t i = 0; i < len; i++) {
        result += hex_chars[data[i] >> 4];
        result += hex_chars[data[i] & 0x0F];
    }

    return result;
}

std::string bytes_to_hex(const std::vector<uint8_t>& data) {
    return bytes_to_hex(data.data(), data.size());
}

bool hex_to_bytes(const std::string& hex, std::vector<uint8_t>& out) {
    if (hex.size() % 2 != 0) {
        return false;
    }

    out.clear();
    out.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        char high = hex[i];
        char low = hex[i + 1];

        uint8_t byte = 0;

        // Parse high nibble
        if (high >= '0' && high <= '9') {
            byte = (high - '0') << 4;
        } else if (high >= 'a' && high <= 'f') {
            byte = (high - 'a' + 10) << 4;
        } else if (high >= 'A' && high <= 'F') {
            byte = (high - 'A' + 10) << 4;
        } else {
            return false;
        }

        // Parse low nibble
        if (low >= '0' && low <= '9') {
            byte |= (low - '0');
        } else if (low >= 'a' && low <= 'f') {
            byte |= (low - 'a' + 10);
        } else if (low >= 'A' && low <= 'F') {
            byte |= (low - 'A' + 10);
        } else {
            return false;
        }

        out.push_back(byte);
    }

    return true;
}

} // namespace doge
