#pragma once

#include "process_memory.hpp"
#include <vector>
#include <cstdint>
#include <string>

class PatternScanner {
public:
    PatternScanner(ProcessMemory* process_memory);
    ~PatternScanner();

    // Pattern scanning
    uint64_t ScanPattern(uint64_t start_address, uint64_t end_address, 
                        const std::string& pattern, const std::string& mask);
    
    // Convenience: scan in module
    uint64_t ScanPatternInModule(const std::string& module_name,
                                 const std::string& pattern, const std::string& mask);

    // Scan for signature with wildcards (0x00 = wildcard)
    uint64_t ScanSignature(uint64_t start_address, uint64_t end_address,
                           const std::vector<uint8_t>& signature,
                           const std::vector<bool>& mask);

    // Pattern: "4C 89 ? ? ? ? ? 48 89" where ? is wildcard
    uint64_t ScanPatternString(uint64_t start_address, uint64_t end_address,
                               const std::string& pattern);

    // Get address relative to RIP (x64 relative addressing)
    uint64_t ResolveRelativeAddress(uint64_t instruction_addr, uint32_t offset_from_instr);

    // Common UE4 patterns for PUBG Mobile
    uint64_t FindGWorld(uint64_t libue4_base, uint64_t libue4_size);
    uint64_t FindGNames(uint64_t libue4_base, uint64_t libue4_size);
    uint64_t FindGUObjectArray(uint64_t libue4_base, uint64_t libue4_size);
    uint64_t FindDecryptionFunction(uint64_t libue4_base, uint64_t libue4_size);

private:
    ProcessMemory* process_memory_;
    std::vector<uint8_t> scan_buffer_;
    static const size_t BUFFER_SIZE = 0x100000;  // 1MB buffer

    bool FillBuffer(uint64_t address, size_t size);
};
