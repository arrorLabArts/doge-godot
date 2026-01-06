#include "doge_wallet.h"
#include "crypto/keypair.h"
#include "crypto/address.h"
#include "crypto/message_signer.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

DogeWallet::DogeWallet() {
}

DogeWallet::~DogeWallet() {
}

void DogeWallet::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate_keypair", "compressed", "mainnet"), &DogeWallet::generate_keypair, DEFVAL(true), DEFVAL(true));
    ClassDB::bind_method(D_METHOD("import_from_wif", "wif"), &DogeWallet::import_from_wif);
    ClassDB::bind_method(D_METHOD("export_to_wif", "private_key_hex", "compressed", "mainnet"), &DogeWallet::export_to_wif, DEFVAL(true), DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_address_from_public_key", "public_key_hex", "mainnet"), &DogeWallet::get_address_from_public_key, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_address_from_wif", "wif"), &DogeWallet::get_address_from_wif);
    ClassDB::bind_method(D_METHOD("sign_message", "message", "private_key_hex", "compressed"), &DogeWallet::sign_message, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("sign_message_wif", "message", "wif"), &DogeWallet::sign_message_wif);
    ClassDB::bind_method(D_METHOD("verify_message", "message", "signature_base64", "address"), &DogeWallet::verify_message);
    ClassDB::bind_method(D_METHOD("validate_address", "address", "mainnet"), &DogeWallet::validate_address, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("bytes_to_hex", "bytes"), &DogeWallet::bytes_to_hex);
    ClassDB::bind_method(D_METHOD("hex_to_bytes", "hex"), &DogeWallet::hex_to_bytes);
}

Dictionary DogeWallet::generate_keypair(bool compressed, bool mainnet) {
    Dictionary result;

    // Generate private key
    std::vector<uint8_t> private_key;
    if (!doge::generate_private_key(private_key)) {
        UtilityFunctions::push_error("Failed to generate private key");
        return result;
    }

    // Derive public key
    std::vector<uint8_t> public_key;
    if (!doge::derive_public_key(private_key, public_key, compressed)) {
        UtilityFunctions::push_error("Failed to derive public key");
        return result;
    }

    // Generate address
    std::string address = doge::public_key_to_address(public_key, mainnet);
    if (address.empty()) {
        UtilityFunctions::push_error("Failed to generate address");
        return result;
    }

    // Export to WIF
    std::string wif = doge::private_key_to_wif(private_key, compressed, mainnet);
    if (wif.empty()) {
        UtilityFunctions::push_error("Failed to export WIF");
        return result;
    }

    result["private_key"] = String(wif.c_str());
    result["public_key"] = String(doge::bytes_to_hex(public_key).c_str());
    result["address"] = String(address.c_str());

    return result;
}

Dictionary DogeWallet::import_from_wif(const String& wif) {
    Dictionary result;

    std::vector<uint8_t> private_key;
    bool compressed;
    bool mainnet;

    std::string wif_str = wif.utf8().get_data();
    if (!doge::wif_to_private_key(wif_str, private_key, compressed, mainnet)) {
        UtilityFunctions::push_error("Invalid WIF private key");
        return result;
    }

    // Derive public key
    std::vector<uint8_t> public_key;
    if (!doge::derive_public_key(private_key, public_key, compressed)) {
        UtilityFunctions::push_error("Failed to derive public key");
        return result;
    }

    // Generate address
    std::string address = doge::public_key_to_address(public_key, mainnet);
    if (address.empty()) {
        UtilityFunctions::push_error("Failed to generate address");
        return result;
    }

    result["private_key"] = wif;
    result["public_key"] = String(doge::bytes_to_hex(public_key).c_str());
    result["address"] = String(address.c_str());

    return result;
}

String DogeWallet::export_to_wif(const String& private_key_hex, bool compressed, bool mainnet) {
    std::vector<uint8_t> private_key;
    std::string hex_str = private_key_hex.utf8().get_data();

    if (!doge::hex_to_bytes(hex_str, private_key)) {
        UtilityFunctions::push_error("Invalid hex private key");
        return String();
    }

    if (private_key.size() != 32) {
        UtilityFunctions::push_error("Private key must be 32 bytes");
        return String();
    }

    std::string wif = doge::private_key_to_wif(private_key, compressed, mainnet);
    if (wif.empty()) {
        UtilityFunctions::push_error("Failed to export WIF");
        return String();
    }

    return String(wif.c_str());
}

String DogeWallet::get_address_from_public_key(const String& public_key_hex, bool mainnet) {
    std::vector<uint8_t> public_key;
    std::string hex_str = public_key_hex.utf8().get_data();

    if (!doge::hex_to_bytes(hex_str, public_key)) {
        UtilityFunctions::push_error("Invalid hex public key");
        return String();
    }

    std::string address = doge::public_key_to_address(public_key, mainnet);
    if (address.empty()) {
        UtilityFunctions::push_error("Failed to generate address");
        return String();
    }

    return String(address.c_str());
}

String DogeWallet::get_address_from_wif(const String& wif) {
    std::string wif_str = wif.utf8().get_data();
    std::string address = doge::wif_to_address(wif_str);

    if (address.empty()) {
        UtilityFunctions::push_error("Failed to get address from WIF");
        return String();
    }

    return String(address.c_str());
}

String DogeWallet::sign_message(const String& message, const String& private_key_hex, bool compressed) {
    std::vector<uint8_t> private_key;
    std::string hex_str = private_key_hex.utf8().get_data();

    if (!doge::hex_to_bytes(hex_str, private_key)) {
        UtilityFunctions::push_error("Invalid hex private key");
        return String();
    }

    if (private_key.size() != 32) {
        UtilityFunctions::push_error("Private key must be 32 bytes");
        return String();
    }

    std::string msg_str = message.utf8().get_data();
    std::string signature = doge::sign_message(msg_str, private_key, compressed);

    if (signature.empty()) {
        UtilityFunctions::push_error("Failed to sign message");
        return String();
    }

    return String(signature.c_str());
}

String DogeWallet::sign_message_wif(const String& message, const String& wif) {
    std::vector<uint8_t> private_key;
    bool compressed;
    bool mainnet;

    std::string wif_str = wif.utf8().get_data();
    if (!doge::wif_to_private_key(wif_str, private_key, compressed, mainnet)) {
        UtilityFunctions::push_error("Invalid WIF private key");
        return String();
    }

    std::string msg_str = message.utf8().get_data();
    std::string signature = doge::sign_message(msg_str, private_key, compressed);

    if (signature.empty()) {
        UtilityFunctions::push_error("Failed to sign message");
        return String();
    }

    return String(signature.c_str());
}

bool DogeWallet::verify_message(const String& message, const String& signature_base64, const String& address) {
    std::string msg_str = message.utf8().get_data();
    std::string sig_str = signature_base64.utf8().get_data();
    std::string addr_str = address.utf8().get_data();

    return doge::verify_message(msg_str, sig_str, addr_str);
}

bool DogeWallet::validate_address(const String& address, bool mainnet) {
    std::string addr_str = address.utf8().get_data();
    return doge::validate_address(addr_str, mainnet);
}

String DogeWallet::bytes_to_hex(const PackedByteArray& bytes) {
    std::vector<uint8_t> data(bytes.size());
    memcpy(data.data(), bytes.ptr(), bytes.size());

    std::string hex = doge::bytes_to_hex(data);
    return String(hex.c_str());
}

PackedByteArray DogeWallet::hex_to_bytes(const String& hex) {
    std::vector<uint8_t> data;
    std::string hex_str = hex.utf8().get_data();

    if (!doge::hex_to_bytes(hex_str, data)) {
        UtilityFunctions::push_error("Invalid hex string");
        return PackedByteArray();
    }

    PackedByteArray result;
    result.resize(data.size());
    memcpy(result.ptrw(), data.data(), data.size());

    return result;
}
