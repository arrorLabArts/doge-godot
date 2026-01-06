#!/usr/bin/env python
import os
import sys

# Try to detect the host platform automatically
if sys.platform.startswith("linux"):
    default_platform = "linux"
elif sys.platform == "darwin":
    default_platform = "macos"
elif sys.platform == "win32" or sys.platform == "msys":
    default_platform = "windows"
else:
    default_platform = "linux"

env = SConscript("godot-cpp/SConstruct")

# Project settings
env.Append(CPPPATH=["src/"])

# Add secp256k1 include path
env.Append(CPPPATH=["thirdparty/secp256k1/include"])

# No external crypto library needed - using standalone SHA256 and RIPEMD160 implementations

# Collect source files
sources = []
sources += Glob("src/*.cpp")
sources += Glob("src/crypto/*.cpp")
sources += Glob("src/utils/*.cpp")

# Link secp256k1
if env["platform"] == "android":
    # For Android, we'll link the prebuilt libsecp256k1.a
    # Note: use arch instead of android_arch for consistency with godot-cpp
    arch = env.get("arch", "arm64")
    # Map arch to Android ABI
    abi_map = {"arm64": "arm64-v8a", "arm32": "armeabi-v7a", "x86_64": "x86_64", "x86": "x86"}
    android_abi = abi_map.get(arch, "arm64-v8a")
    secp_lib_path = f"thirdparty/secp256k1/build-android-{android_abi}/lib"

    if os.path.exists(secp_lib_path):
        env.Append(LIBPATH=[secp_lib_path])
        env.Append(LIBS=["secp256k1"])
    else:
        print(f"Warning: libsecp256k1.a not found at {secp_lib_path}")
        print("You need to build libsecp256k1 for Android first.")
        print("See build instructions in README.md")

elif env["platform"] == "ios":
    # For iOS, we'll link the prebuilt libsecp256k1.a
    arch = env.get("arch", "arm64")
    secp_lib_path = f"thirdparty/secp256k1/build-ios-{arch}"

    if os.path.exists(secp_lib_path):
        env.Append(LIBPATH=[secp_lib_path])
        env.Append(LIBS=["secp256k1"])
    else:
        print(f"Warning: libsecp256k1.a not found at {secp_lib_path}")
        print("You need to build libsecp256k1 for iOS first.")
        print("See build instructions in README.md")

else:
    # Desktop platforms - try to find system secp256k1 or build locally
    secp_lib_path = "thirdparty/secp256k1/build/.libs"
    if os.path.exists(secp_lib_path):
        env.Append(LIBPATH=[secp_lib_path])
        env.Append(LIBS=["secp256k1"])
    else:
        print("Warning: libsecp256k1 not found. Building it...")
        # Build secp256k1 for desktop
        os.system("cd thirdparty/secp256k1 && ./autogen.sh && ./configure --enable-module-recovery && make")
        env.Append(LIBPATH=[secp_lib_path])
        env.Append(LIBS=["secp256k1"])

# Build the library
if env["platform"] == "android":
    android_arch = env.get("android_arch", "arm64-v8a")
    library = env.SharedLibrary(
        f"demo/addons/doge_wallet/bin/libdogewallet.android.{env['target']}.{android_arch}.so",
        source=sources,
    )
elif env["platform"] == "ios":
    arch = env.get("arch", "arm64")
    simulator = env.get("ios_simulator", False)
    suffix = ".simulator" if simulator else ""
    library = env.SharedLibrary(
        f"demo/addons/doge_wallet/bin/libdogewallet.ios.{env['target']}.{arch}{suffix}.dylib",
        source=sources,
    )
elif env["platform"] == "macos":
    library = env.SharedLibrary(
        f"demo/addons/doge_wallet/bin/libdogewallet.macos.{env['target']}.framework/libdogewallet.macos.{env['target']}",
        source=sources,
    )
else:
    # Linux or Windows
    library = env.SharedLibrary(
        f"demo/addons/doge_wallet/bin/libdogewallet.{env['platform']}.{env['target']}.{env['arch']}",
        source=sources,
    )

Default(library)
