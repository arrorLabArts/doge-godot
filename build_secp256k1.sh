#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -d "thirdparty/secp256k1" ]; then
    print_error "thirdparty/secp256k1 directory not found!"
    print_error "Please run this script from the project root directory."
    exit 1
fi

cd thirdparty/secp256k1

# Build for Android
build_android() {
    local ABI=$1
    print_info "Building libsecp256k1 for Android ${ABI}..."

    if [ -z "$ANDROID_NDK_ROOT" ]; then
        print_error "ANDROID_NDK_ROOT environment variable not set!"
        print_error "Please set it to your Android NDK path."
        exit 1
    fi

    BUILD_DIR="build-android-${ABI}"
    mkdir -p "$BUILD_DIR"

    cmake -B "$BUILD_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$ABI" \
        -DANDROID_PLATFORM=21 \
        -DSECP256K1_ENABLE_MODULE_RECOVERY=ON \
        -DSECP256K1_BUILD_TESTS=OFF \
        -DSECP256K1_BUILD_EXHAUSTIVE_TESTS=OFF \
        -DSECP256K1_BUILD_BENCHMARK=OFF \
        -DBUILD_SHARED_LIBS=OFF

    cmake --build "$BUILD_DIR" --config Release

    if [ -f "$BUILD_DIR/libsecp256k1.a" ]; then
        print_info "✓ Successfully built libsecp256k1 for Android ${ABI}"
    else
        print_error "✗ Failed to build libsecp256k1 for Android ${ABI}"
        exit 1
    fi
}

# Build for iOS
build_ios() {
    local ARCH=$1
    print_info "Building libsecp256k1 for iOS ${ARCH}..."

    # Check if we're on macOS
    if [[ "$OSTYPE" != "darwin"* ]]; then
        print_error "iOS builds require macOS!"
        exit 1
    fi

    BUILD_DIR="build-ios-${ARCH}"
    mkdir -p "$BUILD_DIR"

    cmake -B "$BUILD_DIR" \
        -GXcode \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_ARCHITECTURES="$ARCH" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
        -DSECP256K1_ENABLE_MODULE_RECOVERY=ON \
        -DSECP256K1_BUILD_TESTS=OFF \
        -DSECP256K1_BUILD_EXHAUSTIVE_TESTS=OFF \
        -DSECP256K1_BUILD_BENCHMARK=OFF \
        -DBUILD_SHARED_LIBS=OFF

    cmake --build "$BUILD_DIR" --config Release

    if [ -f "$BUILD_DIR/Release-iphoneos/libsecp256k1.a" ] || [ -f "$BUILD_DIR/Release/libsecp256k1.a" ]; then
        print_info "✓ Successfully built libsecp256k1 for iOS ${ARCH}"
    else
        print_error "✗ Failed to build libsecp256k1 for iOS ${ARCH}"
        exit 1
    fi
}

# Parse command line arguments
if [ $# -eq 0 ]; then
    print_error "No platform specified!"
    echo "Usage: $0 <platform> [architectures...]"
    echo ""
    echo "Platforms:"
    echo "  android - Build for Android (requires ANDROID_NDK_ROOT)"
    echo "  ios     - Build for iOS (requires macOS)"
    echo "  all     - Build for all supported platforms"
    echo ""
    echo "Examples:"
    echo "  $0 android arm64-v8a"
    echo "  $0 android arm64-v8a armeabi-v7a"
    echo "  $0 ios arm64"
    echo "  $0 all"
    exit 1
fi

PLATFORM=$1
shift

case $PLATFORM in
    android)
        if [ $# -eq 0 ]; then
            # Default to arm64-v8a
            build_android "arm64-v8a"
        else
            # Build for specified architectures
            for ABI in "$@"; do
                build_android "$ABI"
            done
        fi
        ;;
    ios)
        if [ $# -eq 0 ]; then
            # Default to arm64
            build_ios "arm64"
        else
            # Build for specified architectures
            for ARCH in "$@"; do
                build_ios "$ARCH"
            done
        fi
        ;;
    all)
        print_info "Building for all supported platforms..."
        if [[ "$OSTYPE" == "darwin"* ]]; then
            build_ios "arm64"
        else
            print_warning "Skipping iOS build (not on macOS)"
        fi

        if [ -n "$ANDROID_NDK_ROOT" ]; then
            build_android "arm64-v8a"
            build_android "armeabi-v7a"
        else
            print_warning "Skipping Android build (ANDROID_NDK_ROOT not set)"
        fi
        ;;
    *)
        print_error "Unknown platform: $PLATFORM"
        exit 1
        ;;
esac

cd ../..

print_info "Build complete!"
print_info "You can now build the GDExtension using SCons."
