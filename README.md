# PUBG Mobile UE4 SDK Dumper

A standalone C++ tool for dumping the Unreal Engine 4 SDK from PUBG Mobile on rooted Android (ARM64) devices.

## Features

- ✅ **External Memory Reading** - No process injection required, reads via `/proc/pid/mem`
- ✅ **Pattern Scanning** - Locates GWorld, GNames, GUObjectArray using pattern matching
- ✅ **Pointer Decryption Detection** - Identifies encryption patterns used by PUBG
- ✅ **Full SDK Export** - Generates:
  - `Offsets.hpp` - Critical offset definitions
  - `SDK.hpp` - Full class definitions with property offsets
  - `dump.json` - Complete JSON dump of all classes and offsets
- ✅ **Root Access Required** - Runs as root for memory access
- ✅ **ARM64 Support** - Compiled for 64-bit ARM architecture
- ✅ **Error Recovery** - Graceful fallback patterns if primary detection fails

## Requirements

- Rooted Android device
- Android NDK for compilation
- PUBG Mobile installed and running
- ARM64 processor

## Building

### Using Android NDK (Recommended)

```bash
# Set NDK path
export NDK_PATH=/path/to/android-ndk

# Build
cd SDK
$NDK_PATH/ndk-build APP_ABI=arm64-v8a

# Binary will be at: libs/arm64-v8a/ue4dumper
```

### Using CMake

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 ..
make
```

## Usage

### 1. Push Binary to Device

```bash
adb push libs/arm64-v8a/ue4dumper /data/local/tmp/
adb shell chmod +x /data/local/tmp/ue4dumper
```

### 2. Start PUBG Mobile

Launch PUBG Mobile on the device and wait for it to fully load.

### 3. Run Dumper as Root

```bash
adb shell
su
cd /data/local/tmp
./ue4dumper
```

### 4. Retrieve Output

```bash
adb pull /sdcard/UE4Dumper/ ./output/
```

## Output Files

All files are saved to `/sdcard/UE4Dumper/`:

- **Offsets.hpp** - C++ header with offset constants
  ```cpp
  namespace Offsets {
      constexpr uint64_t GWorld = 0x...;
      constexpr uint64_t GNames = 0x...;
      constexpr uint64_t GUObjectArray = 0x...;
  }
  ```

- **SDK.hpp** - Full class definitions with property offsets
  ```cpp
  class ACharacter {
      // Property offsets calculated
  };
  ```

- **dump.json** - Complete JSON dump
  ```json
  {
    "version": "1.0",
    "game": "PUBG Mobile",
    "globals": {
      "GWorld": "0x...",
      "GNames": "0x...",
      "GUObjectArray": "0x..."
    },
    "object_count": 123456,
    "offsets": { ... }
  }
  ```

- **dumper.log** - Detailed logging output

## Architecture

### Core Components

1. **ProcessMemory** (`process_memory.cpp`)
   - External process memory reading via `/proc/pid/mem`
   - Module enumeration and base address resolution
   - Supports both reading and writing

2. **PatternScanner** (`pattern_scanner.cpp`)
   - Signature and pattern matching
   - Optimized buffer-based scanning
   - Built-in UE4-specific patterns

3. **SDKDumper** (`dumper.cpp`)
   - Orchestrates the dumping process
   - GWorld/GNames/GUObjectArray detection
   - Object iteration and class analysis
   - Output file generation

4. **Logger** (`logger.cpp`)
   - Dual logging to logcat and file
   - Color-coded severity levels
   - Hex dump utilities

### Pattern Scanning Strategy

The tool uses multiple fallback patterns:

1. **Primary Pattern** - Most common in current PUBG version
2. **Secondary Pattern** - Alternative instruction sequences
3. **Tertiary Pattern** - Older version compatibility
4. **Function Signature** - Last resort detection

## Technical Details

### Memory Layout

```
libUE4.so
├── GWorld (0x...)
├── GNames (0x...)
├── GUObjectArray (0x...)
└── [Classes...]
    ├── UObject
    ├── UClass
    ├── UProperty
    └── UStruct
```

### Pointer Decryption

PUBG Mobile may encrypt certain pointers. The tool:
1. Scans for decryption function prologue
2. Analyzes XOR/shift operations
3. Attempts pointer decryption in real-time

### Object Iteration

```cpp
// Simplified iteration logic
for (uint64_t i = 0; i < max_objects; ++i) {
    FUObjectItem item = GUObjectArray[i];
    UObject* obj = item.GetObject();
    // Process object...
}
```

## Troubleshooting

### "Failed to find process: com.tencent.ig"
- PUBG Mobile is not running
- Package name may differ by region (check `adb shell pm list packages | grep tencent`)

### "libUE4.so not found"
- PUBG hasn't fully loaded yet
- Wait 10-15 seconds before running

### "GWorld pattern not found"
- Different PUBG version with different offsets
- Update pattern database or report issue

### "Permission denied"
- Tool must be run as root (`su` required)
- Device must be rooted

### "Cannot write to /sdcard/UE4Dumper"
- Create directory manually: `adb shell mkdir /sdcard/UE4Dumper`
- Check permissions: `adb shell chmod 777 /sdcard/UE4Dumper`

## Performance

- **Scanning Time**: ~30-60 seconds (depends on libUE4.so size)
- **Object Dumping**: ~5-10 minutes (200K+ objects)
- **Total Time**: ~10-15 minutes
- **Memory Usage**: ~50-100 MB

## Limitations

- Requires rooted device (security requirement)
- ARM64 only (no 32-bit support)
- Pattern-based detection may fail on significantly modified versions
- Some encrypted pointers may not be decryptable
- Limited to external memory reading (no code injection)

## Future Improvements

- [ ] Support for multiple UE4 versions
- [ ] Automatic pattern generation from binary analysis
- [ ] IL2CPP analysis for managed code
- [ ] Export to IDA/Ghidra formats
- [ ] Real-time offset validation
- [ ] Support for ARM32 (32-bit)

## Legal Notice

This tool is provided for **educational and research purposes only**. Unauthorized access to computer systems is illegal. Always ensure you have proper authorization before using this tool on any device or game you do not own.

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## Support

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check existing issues first
- Include device info, PUBG version, and logcat output

## Changelog

### v1.0 (2026-06-06)
- Initial release
- Basic SDK dumping functionality
- Pattern scanning for critical pointers
- JSON and C++ header output
