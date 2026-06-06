#!/bin/bash
# Quick build script for UE4 Dumper

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}PUBG Mobile UE4 SDK Dumper - Build Script${NC}"
echo -e "${GREEN}========================================${NC}"

# Check NDK path
if [ -z "$NDK_PATH" ]; then
    echo -e "${RED}Error: NDK_PATH not set${NC}"
    echo "Usage: export NDK_PATH=/path/to/android-ndk && bash build.sh"
    exit 1
fi

if [ ! -f "$NDK_PATH/ndk-build" ]; then
    echo -e "${RED}Error: ndk-build not found at $NDK_PATH${NC}"
    exit 1
fi

echo -e "${YELLOW}NDK Path: $NDK_PATH${NC}"

# Build
echo -e "${YELLOW}Building for ARM64...${NC}"
$NDK_PATH/ndk-build APP_ABI=arm64-v8a APP_PLATFORM=android-21 -j$(nproc)

# Verify output
if [ -f "libs/arm64-v8a/ue4dumper" ]; then
    SIZE=$(du -h "libs/arm64-v8a/ue4dumper" | cut -f1)
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo -e "${GREEN}Binary: libs/arm64-v8a/ue4dumper (${SIZE})${NC}"
else
    echo -e "${RED}✗ Build failed - binary not found${NC}"
    exit 1
fi

# Optional: Show file info
if command -v file &> /dev/null; then
    echo -e "${YELLOW}File Info:${NC}"
    file "libs/arm64-v8a/ue4dumper"
fi

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Next step: adb push libs/arm64-v8a/ue4dumper /data/local/tmp/${NC}"
echo -e "${GREEN}========================================${NC}"
