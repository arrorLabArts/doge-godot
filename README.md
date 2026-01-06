# Doge Wallet GDExtension

A Godot 4.x GDExtension for Dogecoin operations, providing keypair generation, address management, and message signing capabilities for Android and iOS platforms.

## Features

- **Keypair Generation**: Generate secure Dogecoin keypairs using libsecp256k1
- **WIF Support**: Import/export private keys in Wallet Import Format
- **Dogecoin Addresses**: Generate and validate Dogecoin addresses (D prefix for mainnet)
- **Message Signing**: Sign and verify messages using Bitcoin/Dogecoin message format
- **Mobile Ready**: Optimized for Android and iOS platforms

## Requirements

### General
- Godot 4.2 or later
- Python 3.6+ (for SCons)
- SCons 4.0+
- CMake 3.22+
- Git

### Android
- Android NDK r23c or later
- Set `ANDROID_NDK_ROOT` environment variable

### iOS
- macOS with Xcode 14+
- iOS SDK 12.0+ (minimum deployment target)

## Building

### 1. Clone and Initialize Submodules

```bash
git clone https://github.com/yourusername/doge-godot.git
cd doge-godot
git submodule update --init --recursive
```

### 2. Build libsecp256k1

#### For Android

```bash
cd thirdparty/secp256k1

# Build for arm64-v8a
mkdir -p build-android-arm64-v8a
cmake -B build-android-arm64-v8a \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=21 \
    -DSECP256K1_ENABLE_MODULE_RECOVERY=ON \
    -DSECP256K1_BUILD_TESTS=OFF \
    -DSECP256K1_BUILD_EXHAUSTIVE_TESTS=OFF \
    -DBUILD_SHARED_LIBS=OFF
cmake --build build-android-arm64-v8a --config Release

# Build for armeabi-v7a (optional, for older devices)
mkdir -p build-android-armeabi-v7a
cmake -B build-android-armeabi-v7a \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=21 \
    -DSECP256K1_ENABLE_MODULE_RECOVERY=ON \
    -DSECP256K1_BUILD_TESTS=OFF \
    -DSECP256K1_BUILD_EXHAUSTIVE_TESTS=OFF \
    -DBUILD_SHARED_LIBS=OFF
cmake --build build-android-armeabi-v7a --config Release

cd ../..
```

#### For iOS

```bash
cd thirdparty/secp256k1

# Build for arm64 (device)
mkdir -p build-ios-arm64
cmake -B build-ios-arm64 \
    -GXcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
    -DSECP256K1_ENABLE_MODULE_RECOVERY=ON \
    -DSECP256K1_BUILD_TESTS=OFF \
    -DSECP256K1_BUILD_EXHAUSTIVE_TESTS=OFF \
    -DBUILD_SHARED_LIBS=OFF
cmake --build build-ios-arm64 --config Release

cd ../..
```

### 3. Build godot-cpp

#### For Android

```bash
cd godot-cpp

# Set your NDK version (check what you have in ~/Android/Sdk/ndk/)
export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/27.0.12077973

# Build for arm64
scons platform=android target=template_release arch=arm64 ndk_version=27.0.12077973
scons platform=android target=template_debug arch=arm64 ndk_version=27.0.12077973

# Build for arm32 (optional, for older devices)
scons platform=android target=template_release arch=arm32 ndk_version=27.0.12077973
scons platform=android target=template_debug arch=arm32 ndk_version=27.0.12077973

cd ..
```

#### For iOS

```bash
cd godot-cpp

# Build for iOS arm64
scons platform=ios target=template_release arch=arm64
scons platform=ios target=template_debug arch=arm64

cd ..
```

### 4. Build the GDExtension

#### For Android

```bash
# Set your NDK version (same as above)
export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/27.0.12077973

# Build for arm64-v8a
scons platform=android target=template_release arch=arm64 ndk_version=27.0.12077973
scons platform=android target=template_debug arch=arm64 ndk_version=27.0.12077973

# Build for armeabi-v7a (optional, for older devices)
scons platform=android target=template_release arch=arm32 ndk_version=27.0.12077973
scons platform=android target=template_debug arch=arm32 ndk_version=27.0.12077973
```

**Note:** Replace `27.0.12077973` with your actual NDK version. Find it by running:
```bash
ls ~/Android/Sdk/ndk/
```

#### For iOS

```bash
# Build for iOS arm64
scons platform=ios target=template_release arch=arm64
scons platform=ios target=template_debug arch=arm64
```

## Using in Your Godot Project

### Step 1: Copy Extension Files

After building, copy the extension files to your Godot project:

```bash
# In your Godot project directory
mkdir -p addons/doge_wallet/bin

# Copy the .gdextension config file
cp demo/addons/doge_wallet/doge_wallet.gdextension addons/doge_wallet/

# Copy the compiled library (Android example)
cp demo/addons/doge_wallet/bin/libdogewallet.android.template_release.arm64-v8a.so \
   addons/doge_wallet/bin/
```

Your project structure should look like:
```
YourGame/
├── project.godot
├── addons/
│   └── doge_wallet/
│       ├── doge_wallet.gdextension
│       └── bin/
│           └── libdogewallet.android.template_release.arm64-v8a.so
├── scenes/
└── scripts/
```

### Step 2: Enable the Extension

The extension loads automatically when you open your project in Godot. Verify it loaded by checking the output console for errors.

### Step 3: Export for Android/iOS

When exporting your game:
1. Open **Project → Export**
2. Select your platform (Android/iOS)
3. The `.gdextension` file automatically includes the native libraries
4. Export your APK/IPA

The extension will be bundled with your game!

## Usage

### Basic Example

```gdscript
extends Node

var wallet: DogeWallet

func _ready():
    # Create wallet instance
    wallet = DogeWallet.new()

    # Generate new keypair
    var keypair = wallet.generate_keypair()
    print("Address: ", keypair.address)
    print("Private Key (WIF): ", keypair.private_key)

    # Sign a message
    var message = "Hello, Dogecoin!"
    var signature = wallet.sign_message_wif(message, keypair.private_key)
    print("Signature: ", signature)

    # Verify signature
    var is_valid = wallet.verify_message(message, signature, keypair.address)
    print("Valid: ", is_valid)
```

### Game Integration Examples

#### Example 1: Player Wallet Manager

```gdscript
extends Node

var wallet: DogeWallet
var player_wif: String = ""

func _ready():
    wallet = DogeWallet.new()

func generate_player_address() -> Dictionary:
    var keypair = wallet.generate_keypair()

    # IMPORTANT: Store private key securely!
    save_private_key_securely(keypair.private_key)

    return {
        "address": keypair.address,
        "public_key": keypair.public_key
    }

func save_private_key_securely(wif_key: String):
    # Use encrypted storage
    var file = FileAccess.open_encrypted_with_pass(
        "user://wallet.dat",
        FileAccess.WRITE,
        "your-strong-password"
    )
    file.store_string(wif_key)
    file.close()

func load_private_key() -> String:
    var file = FileAccess.open_encrypted_with_pass(
        "user://wallet.dat",
        FileAccess.READ,
        "your-strong-password"
    )
    var wif = file.get_as_text()
    file.close()
    return wif
```

#### Example 2: Sign Game Achievements

```gdscript
func sign_achievement(level: int, score: int, time: float) -> Dictionary:
    var wif = load_private_key()
    var message = "Level:%d|Score:%d|Time:%.2f" % [level, score, time]
    var signature = wallet.sign_message_wif(message, wif)

    return {
        "player": get_player_address(),
        "proof": message,
        "signature": signature
    }

func verify_achievement(data: Dictionary) -> bool:
    return wallet.verify_message(
        data.proof,
        data.signature,
        data.player
    )
```

#### Example 3: Player Authentication

```gdscript
func authenticate_player() -> Dictionary:
    var challenge = "Login-" + str(Time.get_unix_time_from_system())
    var wif = load_private_key()
    var signature = wallet.sign_message_wif(challenge, wif)

    # Send to game server for verification
    return {
        "address": get_player_address(),
        "challenge": challenge,
        "signature": signature
    }
```

## API Reference

### DogeWallet Class

#### Methods

##### `generate_keypair(compressed: bool = true, mainnet: bool = true) -> Dictionary`

Generate a new random keypair.

**Returns:**
```gdscript
{
    "private_key": String,  # WIF format (Q... for mainnet)
    "public_key": String,   # Hex format
    "address": String       # Dogecoin address (D... for mainnet)
}
```

##### `import_from_wif(wif: String) -> Dictionary`

Import keypair from WIF private key.

**Returns:** Same as `generate_keypair()`

##### `export_to_wif(private_key_hex: String, compressed: bool = true, mainnet: bool = true) -> String`

Convert hex private key to WIF format.

##### `get_address_from_public_key(public_key_hex: String, mainnet: bool = true) -> String`

Generate Dogecoin address from public key.

##### `get_address_from_wif(wif: String) -> String`

Get Dogecoin address from WIF private key.

##### `sign_message(message: String, private_key_hex: String, compressed: bool = true) -> String`

Sign a message with hex private key. Returns base64 signature.

##### `sign_message_wif(message: String, wif: String) -> String`

Sign a message with WIF private key. Returns base64 signature.

##### `verify_message(message: String, signature_base64: String, address: String) -> bool`

Verify a message signature.

##### `validate_address(address: String, mainnet: bool = true) -> bool`

Validate Dogecoin address format.

## Security Considerations

⚠️ **Important Security Notes:**

1. **Private Key Storage**: This extension does NOT handle key storage. You must implement secure storage yourself (use OS keychain, encrypted storage, etc.)

2. **Memory Security**: Private keys are cleared from memory after use in C++ code, but GDScript strings may persist. Minimize time private keys are held in variables.

3. **Random Number Generation**: Uses platform-specific secure RNG:
   - Android: `/dev/urandom` (works on all API levels)
   - iOS: `SecRandomCopyBytes()`
   - Linux: `/dev/urandom`

4. **Production Use**: This is cryptographic software. Audit the code before using with real funds.

### Best Practices for Game Integration

**✅ DO:**
- Use `FileAccess.open_encrypted_with_pass()` for storing private keys
- Implement user-chosen passwords or device-specific encryption
- Clear sensitive data from variables after use: `wif = ""; keypair = {}`
- Consider using Android Keystore or iOS Keychain via plugins for production
- Validate all user inputs before signing
- Use timeouts/nonces in authentication to prevent replay attacks

**❌ DON'T:**
- Store private keys in plain text files
- Hardcode encryption passwords in your game
- Send private keys over the network (only send signatures)
- Store private keys in game save files without encryption
- Log private keys to console in production builds

**Example: Secure Storage Pattern**
```gdscript
# Get device-specific key (simplified example)
func get_device_key() -> String:
    return OS.get_unique_id().sha256_text().substr(0, 32)

func save_wallet(wif: String):
    var password = get_device_key()
    var file = FileAccess.open_encrypted_with_pass(
        "user://wallet.dat",
        FileAccess.WRITE,
        password
    )
    file.store_string(wif)
    file.close()
```

## Testing

### Testing the Demo Project

Run the demo project in Godot to test all functionality:

```bash
cd demo
godot --path . main.tscn
```

### Testing in Your Game

Create a simple test scene to verify the extension loaded:

```gdscript
extends Node

func _ready():
    var wallet = DogeWallet.new()
    if wallet:
        print("✓ DogeWallet loaded successfully!")
        _test_basic_functions(wallet)
    else:
        push_error("✗ Failed to load DogeWallet extension")

func _test_basic_functions(wallet: DogeWallet):
    # Test keypair generation
    var kp = wallet.generate_keypair()
    assert(kp.address.begins_with("D"), "Address should start with D")
    print("✓ Address generated: ", kp.address)

    # Test signing
    var sig = wallet.sign_message_wif("test", kp.private_key)
    assert(sig.length() > 0, "Signature should not be empty")
    print("✓ Message signed")

    # Test verification
    var valid = wallet.verify_message("test", sig, kp.address)
    assert(valid, "Signature should be valid")
    print("✓ Signature verified")

    print("All tests passed!")
```

### Debugging on Android

If the extension doesn't load on Android:

```bash
# Check logcat for errors
adb logcat | grep -i "doge\|gdextension\|GDNative"

# Verify the .so file is in the APK
unzip -l your_game.apk | grep dogewallet

# Check file permissions
adb shell ls -la /data/app/*/lib/arm64/
```

## Technical Details

### Dogecoin Specifics

- **Address Version**: `0x1e` (mainnet, produces 'D' prefix)
- **WIF Version**: `0x9e` (mainnet, produces 'Q' prefix)
- **Message Signing**: Uses Bitcoin message format with `\x18Bitcoin Signed Message:\n` prefix

### Dependencies

- **godot-cpp**: Official C++ bindings for Godot
- **libsecp256k1**: Bitcoin's official elliptic curve library (with recovery module)
- **Standalone crypto**: SHA256 and RIPEMD160 implemented without external dependencies (no OpenSSL required)

### Build Artifacts

After building, you'll find the extension libraries in:
- `demo/addons/doge_wallet/bin/libdogewallet.android.*.so` (Android)
- `demo/addons/doge_wallet/bin/libdogewallet.ios.*.dylib` (iOS)

## License

MIT License - see LICENSE file for details

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Submit a pull request

## Support

For issues and questions:
- GitHub Issues: https://github.com/yourusername/doge-godot/issues
- Dogecoin documentation: https://dogecoin.com

## Acknowledgments

- Bitcoin Core team for libsecp256k1
- Godot Engine team for GDExtension API
- Dogecoin community

---

**Disclaimer**: This software is provided as-is. Use at your own risk. Always test thoroughly before using with real cryptocurrency.
