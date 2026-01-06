#ifndef DOGE_WALLET_H
#define DOGE_WALLET_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class DogeWallet : public RefCounted {
    GDCLASS(DogeWallet, RefCounted)

protected:
    static void _bind_methods();

public:
    DogeWallet();
    ~DogeWallet();

    // Generate a new keypair
    // Returns: {private_key: String (WIF), public_key: String (hex), address: String}
    Dictionary generate_keypair(bool compressed = true, bool mainnet = true);

    // Import keypair from WIF private key
    // Returns: {private_key: String (WIF), public_key: String (hex), address: String}
    Dictionary import_from_wif(const String& wif);

    // Export private key to WIF format
    String export_to_wif(const String& private_key_hex, bool compressed = true, bool mainnet = true);

    // Get Dogecoin address from public key (hex)
    String get_address_from_public_key(const String& public_key_hex, bool mainnet = true);

    // Get Dogecoin address from WIF private key
    String get_address_from_wif(const String& wif);

    // Sign a message with private key (hex format)
    String sign_message(const String& message, const String& private_key_hex, bool compressed = true);

    // Sign a message with WIF private key
    String sign_message_wif(const String& message, const String& wif);

    // Verify a message signature
    bool verify_message(const String& message, const String& signature_base64, const String& address);

    // Validate Dogecoin address format
    bool validate_address(const String& address, bool mainnet = true);

    // Utility: Convert hex to bytes and vice versa
    String bytes_to_hex(const PackedByteArray& bytes);
    PackedByteArray hex_to_bytes(const String& hex);
};

#endif // DOGE_WALLET_H
