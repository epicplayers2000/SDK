# SDK Dumper - Frequently Asked Questions

## Building & Compilation

### Q: "ndk-build: command not found"
**A:** Add NDK to PATH:
```bash
export PATH=$PATH:$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin
```

### Q: Build error: "jni.h not found"
**A:** NDK path is incorrect or incomplete. Verify:
```bash
ls $NDK_PATH/toolchains/llvm/prebuilt/*/sysroot/usr/include/jni.h
```

### Q: What NDK version is needed?
**A:** r25 LTS or newer. Older versions may have compatibility issues.

### Q: Can I cross-compile on macOS or Windows?
**A:** Yes. Download NDK for your platform. The build process is identical.

---

## Deployment & Execution

### Q: "Failed to attach to process"
**A:** 
1. PUBG Mobile is not running - launch the game first
2. Package name might differ by region:
   ```bash
   adb shell pm list packages | grep tencent
   ```
3. Edit `src/main.cpp` and update `com.tencent.ig`

### Q: "Permission denied" when executing
**A:** Binary must be run as root:
```bash
adb shell
su
/data/local/tmp/ue4dumper
```

### Q: Process crashes immediately
**A:** 
1. Device must be rooted (`su` must work)
2. Try with: `adb shell "su -c /data/local/tmp/ue4dumper"`
3. Check logcat: `adb logcat | grep UE4Dumper`

---

## Pattern Scanning Issues

### Q: "GWorld pattern not found"
**A:** 
1. PUBG version mismatch - different instruction sequences
2. Try waiting longer for game to fully load
3. Update patterns in `src/memory/pattern_scanner.cpp`

### Q: "libUE4.so not found"
**A:** 
1. Game hasn't loaded - wait 15+ seconds
2. Some regions use different library names
3. Check: `adb shell ls /data/data/com.tencent.ig/lib/`

### Q: Multiple pattern matches found - which is correct?
**A:** The first match is used. If incorrect, add more specific pattern or add validation logic.

---

## Output & Results

### Q: No output files generated
**A:** Check permissions:
```bash
adb shell ls -la /sdcard/UE4Dumper/
adb shell cat /sdcard/UE4Dumper/dumper.log
```

### Q: Output shows zeros (0x00000000)
**A:** Pattern scanning failed. Try:
1. Waiting longer for game initialization
2. Using a different PUBG version
3. Adding debug logging to pattern scanner

### Q: "Cannot write to /sdcard/"
**A:** Create and chmod directory:
```bash
adb shell mkdir -p /sdcard/UE4Dumper
adb shell chmod 777 /sdcard/UE4Dumper
```

---

## Memory & Performance

### Q: "Out of memory" error
**A:** 
1. Close other apps on device
2. Reduce pattern scan buffer size in `pattern_scanner.hpp`
3. Reduce max objects in `dumper.cpp` (currently 200K)

### Q: Dumping is very slow (>30 minutes)
**A:** 
1. Normal if device is slow - ARM Cortex-A73 and slower
2. Reduce object count: edit `dumper.cpp` line `uint64_t max_objects = 200000;`
3. Use faster storage (internal vs SD card)

### Q: How much disk space needed?
**A:** 
- Binary: ~800KB
- Output files: ~50-100MB (depending on object count)

---

## Advanced Usage

### Q: Can I modify the tool for different games?
**A:** Yes:
1. Change process name in `main.cpp`
2. Update UE4 type offsets in `ue_types.hpp`
3. Modify pattern scanning in `pattern_scanner.cpp`

### Q: How do I add custom pattern matching?
**A:** 
```cpp
// In pattern_scanner.cpp
uint64_t PatternScanner::FindCustomPointer(...) {
    return ScanPatternInModule("libUE4.so", "48 8D 15", "xxx");
}
```

### Q: Can I write to process memory (not just read)?
**A:** Yes, already implemented:
```cpp
ProcessMemory::WriteMemory(address, data, size);
```

### Q: How do I export to IDA or Ghidra?
**A:** Parse JSON output and convert to appropriate format:
- IDA: Generate `.idc` script
- Ghidra: Convert to `.gdt` database

---

## Debugging

### Q: How do I enable debug logging?
**A:** Edit `Android.mk`:
```makefile
LOCAL_CFLAGS := -DDEBUG_MODE -g
```

### Q: Where are logs stored?
**A:** 
1. Logcat: `adb logcat | grep UE4Dumper`
2. File: `/sdcard/UE4Dumper/dumper.log`

### Q: Can I trace execution with GDB?
**A:** Yes, with debug binary:
```bash
adb shell gdbserver :5005 /data/local/tmp/ue4dumper
# In another terminal:
arm-linux-android-gdb -ex "target remote :5005"
```

---

## Compatibility

### Q: Which PUBG Mobile versions work?
**A:** Tested on 2.0+ (June 2026). Older versions may need pattern updates.

### Q: Can this work on 32-bit devices?
**A:** No, compiled for ARM64 only. Would need separate ARM32 build.

### Q: What about iOS version?
**A:** iOS uses different memory model (no `/proc/`). Would need completely different approach.

---

## Legal & Safety

### Q: Is this tool legal?
**A:** 
- Reverse engineering for research: Generally legal in most jurisdictions
- Check your local laws and game ToS
- This tool is for educational purposes

### Q: Will this get me banned from PUBG?
**A:** 
- Tool doesn't modify game
- External memory reading only
- Risk exists - use at own discretion
- Test on secondary account first

### Q: Can I use this publicly?
**A:** Distribute at own risk. Consider legal implications and ToS violations.

---

## Performance Tips

### Q: How to make dumping faster?
1. **Disable logging**: Remove file writes
2. **Reduce scan buffer**: 512KB instead of 1MB
3. **Limit objects**: Scan first 100K instead of 200K
4. **Use fast storage**: Internal storage vs SD card

### Q: What's the slowest part?
**A:** Object iteration (~80% of time). Mainly due to external memory reads.

---

## Troubleshooting Checklist

Before opening an issue, verify:

- [ ] Device is rooted (`su` works)
- [ ] PUBG Mobile is running and fully loaded
- [ ] Binary was built successfully
- [ ] Binary has execute permissions
- [ ] Device has free storage (>200MB)
- [ ] PUBG package name is correct
- [ ] Waited 15+ seconds after launching game
- [ ] Tried on different device/version if possible

---

## Contact & Support

- **GitHub Issues**: https://github.com/epicplayers2000/SDK/issues
- **Discussions**: https://github.com/epicplayers2000/SDK/discussions

Include:
- Device model and Android version
- PUBG Mobile version
- Error message and logcat output
- Steps to reproduce
