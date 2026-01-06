#ifndef DOGE_MESSAGE_SIGNER_H
#define DOGE_MESSAGE_SIGNER_H

#include <string>
#include <vector>
#include <cstdint>

namespace doge {

// Sign a message using Bitcoin message signing format
// Returns base64-encoded signature (65 bytes: recovery_id + r + s)
std::string sign_message(const std::string& message,
                         const std::vector<uint8_t>& private_key,
                         bool compressed = true);

// Verify a message signature
// Returns true if signature is valid for the given message and address
bool verify_message(const std::string& message,
                    const std::string& signature_base64,
                    const std::string& address);

// Base64 encoding/decoding helpers
std::string base64_encode(const uint8_t* data, size_t len);
std::string base64_encode(const std::vector<uint8_t>& data);
bool base64_decode(const std::string& str, std::vector<uint8_t>& out);

} // namespace doge

#endif // DOGE_MESSAGE_SIGNER_H
