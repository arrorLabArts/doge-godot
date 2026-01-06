#include "message_signer.h"
#include "address.h"
#include "../utils/hash.h"
#include <secp256k1.h>
#include <secp256k1_recovery.h>
#include <cstring>
#include <sstream>

namespace doge {

static secp256k1_context* get_secp256k1_context() {
    static secp256k1_context* ctx = nullptr;
    if (!ctx) {
        ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    }
    return ctx;
}

// Encode varint (variable-length integer)
static void encode_varint(size_t value, std::vector<uint8_t>& out) {
    if (value < 0xfd) {
        out.push_back(static_cast<uint8_t>(value));
    } else if (value <= 0xffff) {
        out.push_back(0xfd);
        out.push_back(static_cast<uint8_t>(value & 0xff));
        out.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
    } else if (value <= 0xffffffff) {
        out.push_back(0xfe);
        out.push_back(static_cast<uint8_t>(value & 0xff));
        out.push_back(static_cast<uint8_t>((value >> 8) & 0xff));
        out.push_back(static_cast<uint8_t>((value >> 16) & 0xff));
        out.push_back(static_cast<uint8_t>((value >> 24) & 0xff));
    } else {
        out.push_back(0xff);
        for (int i = 0; i < 8; i++) {
            out.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xff));
        }
    }
}

// Construct Bitcoin message format for signing
static void construct_bitcoin_message(const std::string& message, std::vector<uint8_t>& out) {
    // Magic string (using octal escape to avoid hex parsing issues)
    const char* magic = "\030Bitcoin Signed Message:\n";
    size_t magic_len = 25; // strlen(magic) = 25

    // Message bytes
    const uint8_t* msg_data = reinterpret_cast<const uint8_t*>(message.data());
    size_t msg_len = message.size();

    // Build: magic + varint(msg_len) + message
    out.clear();
    out.insert(out.end(), magic, magic + magic_len);

    std::vector<uint8_t> varint;
    encode_varint(msg_len, varint);
    out.insert(out.end(), varint.begin(), varint.end());

    out.insert(out.end(), msg_data, msg_data + msg_len);
}

std::string sign_message(const std::string& message,
                         const std::vector<uint8_t>& private_key,
                         bool compressed) {
    if (private_key.size() != 32) {
        return "";
    }

    // Construct Bitcoin message format
    std::vector<uint8_t> full_message;
    construct_bitcoin_message(message, full_message);

    // Double SHA256 hash
    uint8_t hash[32];
    sha256_double(full_message.data(), full_message.size(), hash);

    // Sign with secp256k1 (recoverable signature)
    secp256k1_context* ctx = get_secp256k1_context();
    secp256k1_ecdsa_recoverable_signature sig;

    if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, hash, private_key.data(), nullptr, nullptr)) {
        return "";
    }

    // Serialize to compact format (64 bytes: r + s) + recovery_id
    uint8_t compact_sig[64];
    int recovery_id;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, compact_sig, &recovery_id, &sig);

    // Build final signature (65 bytes)
    // First byte: 27 + recovery_id + (4 if compressed)
    std::vector<uint8_t> signature(65);
    signature[0] = 27 + recovery_id + (compressed ? 4 : 0);
    memcpy(signature.data() + 1, compact_sig, 64);

    return base64_encode(signature);
}

bool verify_message(const std::string& message,
                    const std::string& signature_base64,
                    const std::string& address) {
    // Decode signature
    std::vector<uint8_t> signature;
    if (!base64_decode(signature_base64, signature)) {
        return false;
    }

    if (signature.size() != 65) {
        return false;
    }

    // Extract recovery_id and compressed flag from first byte
    uint8_t header = signature[0];
    if (header < 27 || header >= 27 + 8) {
        return false;
    }

    int recovery_id = (header - 27) & 3;
    bool compressed = (header - 27) >= 4;

    // Construct Bitcoin message format
    std::vector<uint8_t> full_message;
    construct_bitcoin_message(message, full_message);

    // Double SHA256 hash
    uint8_t hash[32];
    sha256_double(full_message.data(), full_message.size(), hash);

    // Parse recoverable signature
    secp256k1_context* ctx = get_secp256k1_context();
    secp256k1_ecdsa_recoverable_signature sig;

    if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &sig, signature.data() + 1, recovery_id)) {
        return false;
    }

    // Recover public key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ecdsa_recover(ctx, &pubkey, &sig, hash)) {
        return false;
    }

    // Serialize public key
    std::vector<uint8_t> pubkey_bytes(compressed ? 33 : 65);
    size_t pubkey_len = pubkey_bytes.size();
    unsigned int flags = compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;

    if (!secp256k1_ec_pubkey_serialize(ctx, pubkey_bytes.data(), &pubkey_len, &pubkey, flags)) {
        return false;
    }

    // Generate address from recovered public key
    std::string recovered_address = public_key_to_address(pubkey_bytes, true);

    // Compare addresses
    return recovered_address == address;
}

// Base64 encoding/decoding
static const char* BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const uint8_t* data, size_t len) {
    std::string result;
    result.reserve(((len + 2) / 3) * 4);

    for (size_t i = 0; i < len; i += 3) {
        uint32_t triple = (data[i] << 16);

        if (i + 1 < len) {
            triple |= (data[i + 1] << 8);
        }
        if (i + 2 < len) {
            triple |= data[i + 2];
        }

        result += BASE64_ALPHABET[(triple >> 18) & 0x3F];
        result += BASE64_ALPHABET[(triple >> 12) & 0x3F];
        result += (i + 1 < len) ? BASE64_ALPHABET[(triple >> 6) & 0x3F] : '=';
        result += (i + 2 < len) ? BASE64_ALPHABET[triple & 0x3F] : '=';
    }

    return result;
}

std::string base64_encode(const std::vector<uint8_t>& data) {
    return base64_encode(data.data(), data.size());
}

bool base64_decode(const std::string& str, std::vector<uint8_t>& out) {
    if (str.empty()) {
        out.clear();
        return true;
    }

    // Remove padding
    size_t len = str.size();
    while (len > 0 && str[len - 1] == '=') {
        len--;
    }

    out.clear();
    out.reserve((len * 3) / 4);

    uint32_t buffer = 0;
    int bits = 0;

    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        int value;

        if (c >= 'A' && c <= 'Z') {
            value = c - 'A';
        } else if (c >= 'a' && c <= 'z') {
            value = c - 'a' + 26;
        } else if (c >= '0' && c <= '9') {
            value = c - '0' + 52;
        } else if (c == '+') {
            value = 62;
        } else if (c == '/') {
            value = 63;
        } else {
            return false; // Invalid character
        }

        buffer = (buffer << 6) | value;
        bits += 6;

        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<uint8_t>((buffer >> bits) & 0xFF));
        }
    }

    return true;
}

} // namespace doge
