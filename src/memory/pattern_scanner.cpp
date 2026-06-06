#include "pattern_scanner.hpp"
#include "logger.hpp"
#include <cstring>
#include <algorithm>

PatternScanner::PatternScanner(ProcessMemory* process_memory)
    : process_memory_(process_memory) {
    scan_buffer_.resize(BUFFER_SIZE);
}

PatternScanner::~PatternScanner() {}

uint64_t PatternScanner::ScanPattern(uint64_t start_address, uint64_t end_address,
                                     const std::string& pattern, const std::string& mask) {
    if (!process_memory_ || !process_memory_->IsAttached()) {
        LOG_ERROR("Process not attached");
        return 0;
    }

    size_t pattern_len = pattern.length();
    if (pattern_len == 0 || pattern_len != mask.length()) {
        return 0;
    }

    uint64_t current_addr = start_address;
    while (current_addr < end_address) {
        size_t bytes_to_read = std::min((size_t)(end_address - current_addr), BUFFER_SIZE);
        
        if (!FillBuffer(current_addr, bytes_to_read)) {
            current_addr += BUFFER_SIZE;
            continue;
        }

        for (size_t i = 0; i + pattern_len <= bytes_to_read; ++i) {
            bool match = true;
            for (size_t j = 0; j < pattern_len; ++j) {
                if (mask[j] == 'x' && scan_buffer_[i + j] != (uint8_t)pattern[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                LOG_INFO("Pattern found at: 0x" + std::to_string(current_addr + i));
                return current_addr + i;
            }
        }

        current_addr += BUFFER_SIZE - pattern_len;
    }

    return 0;
}

uint64_t PatternScanner::ScanPatternInModule(const std::string& module_name,
                                            const std::string& pattern, const std::string& mask) {
    uint64_t module_base = process_memory_->GetModuleBase(module_name);
    if (module_base == 0) {
        LOG_ERROR("Module not found: " + module_name);
        return 0;
    }

    // Get module size from maps
    auto modules = process_memory_->GetModules();
    uint64_t module_end = 0;

    for (const auto& mod : modules) {
        if (mod.first.find(module_name) != std::string::npos) {
            // For simplicity, assume 100MB max size
            module_end = mod.second + (100 * 1024 * 1024);
            break;
        }
    }

    if (module_end == 0) {
        return 0;
    }

    LOG_INFO("Scanning module " + module_name + " at 0x" + std::to_string(module_base));
    return ScanPattern(module_base, module_end, pattern, mask);
}

uint64_t PatternScanner::ScanSignature(uint64_t start_address, uint64_t end_address,
                                       const std::vector<uint8_t>& signature,
                                       const std::vector<bool>& mask) {
    if (signature.size() != mask.size()) {
        return 0;
    }

    uint64_t current_addr = start_address;
    size_t sig_len = signature.size();

    while (current_addr < end_address) {
        size_t bytes_to_read = std::min((size_t)(end_address - current_addr), BUFFER_SIZE);
        
        if (!FillBuffer(current_addr, bytes_to_read)) {
            current_addr += BUFFER_SIZE;
            continue;
        }

        for (size_t i = 0; i + sig_len <= bytes_to_read; ++i) {
            bool match = true;
            for (size_t j = 0; j < sig_len; ++j) {
                if (mask[j] && scan_buffer_[i + j] != signature[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                return current_addr + i;
            }
        }

        current_addr += BUFFER_SIZE - sig_len;
    }

    return 0;
}

uint64_t PatternScanner::ScanPatternString(uint64_t start_address, uint64_t end_address,
                                           const std::string& pattern) {
    std::vector<uint8_t> sig;
    std::vector<bool> mask;

    // Parse pattern string: "4C 89 ? ? ? ?"
    size_t i = 0;
    while (i < pattern.length()) {
        while (i < pattern.length() && (pattern[i] == ' ' || pattern[i] == '\t')) {
            ++i;
        }

        if (i >= pattern.length()) break;

        if (pattern[i] == '?' && (i + 1 >= pattern.length() || pattern[i + 1] == ' ' || pattern[i + 1] == '?')) {
            sig.push_back(0);
            mask.push_back(false);
            ++i;
        } else if (isxdigit(pattern[i]) && i + 1 < pattern.length() && isxdigit(pattern[i + 1])) {
            std::string hex_byte = pattern.substr(i, 2);
            sig.push_back((uint8_t)std::stoi(hex_byte, nullptr, 16));
            mask.push_back(true);
            i += 2;
        } else {
            ++i;
        }
    }

    return ScanSignature(start_address, end_address, sig, mask);
}

uint64_t PatternScanner::ResolveRelativeAddress(uint64_t instruction_addr, uint32_t offset_from_instr) {
    // For x64 RIP-relative addressing: address = instruction_addr + instruction_length + offset
    // This assumes the offset is already calculated
    return instruction_addr + offset_from_instr;
}

uint64_t PatternScanner::FindGWorld(uint64_t libue4_base, uint64_t libue4_size) {
    // Pattern for GWorld access in UE4
    // This varies by version, trying common patterns
    
    LOG_INFO("Searching for GWorld...");
    
    // Pattern 1: lea rax, [rel GWorld]
    uint64_t result = ScanPattern(libue4_base, libue4_base + libue4_size,
                                  "\x48\x8D\x05", "xxx");
    if (result) {
        LOG_INFO("Found GWorld pattern at: 0x" + std::to_string(result));
        return result;
    }

    // Pattern 2: mov rax, [rel GWorld]
    result = ScanPattern(libue4_base, libue4_base + libue4_size,
                        "\x48\x8B\x05", "xxx");
    if (result) {
        LOG_INFO("Found GWorld pattern at: 0x" + std::to_string(result));
        return result;
    }

    LOG_WARNING("GWorld pattern not found");
    return 0;
}

uint64_t PatternScanner::FindGNames(uint64_t libue4_base, uint64_t libue4_size) {
    LOG_INFO("Searching for GNames...");
    
    // Pattern: lea rax, [rel GNames]
    uint64_t result = ScanPattern(libue4_base, libue4_base + libue4_size,
                                  "\x48\x8D\x05", "xxx");
    if (result) {
        LOG_INFO("Found GNames pattern at: 0x" + std::to_string(result));
        return result;
    }

    LOG_WARNING("GNames pattern not found");
    return 0;
}

uint64_t PatternScanner::FindGUObjectArray(uint64_t libue4_base, uint64_t libue4_size) {
    LOG_INFO("Searching for GUObjectArray...");
    
    // Pattern: mov rcx, [rel GUObjectArray]
    uint64_t result = ScanPattern(libue4_base, libue4_base + libue4_size,
                                  "\x48\x8B\x0D", "xxx");
    if (result) {
        LOG_INFO("Found GUObjectArray pattern at: 0x" + std::to_string(result));
        return result;
    }

    LOG_WARNING("GUObjectArray pattern not found");
    return 0;
}

uint64_t PatternScanner::FindDecryptionFunction(uint64_t libue4_base, uint64_t libue4_size) {
    LOG_INFO("Searching for pointer decryption function...");
    
    // This is game-specific, pattern may vary
    // Looking for typical decryption prologue
    uint64_t result = ScanPattern(libue4_base, libue4_base + libue4_size,
                                  "\x55\x48\x89\xE5", "xxxx");
    if (result) {
        LOG_INFO("Found potential decryption function at: 0x" + std::to_string(result));
        return result;
    }

    LOG_WARNING("Decryption function not found");
    return 0;
}

bool PatternScanner::FillBuffer(uint64_t address, size_t size) {
    if (size > BUFFER_SIZE) {
        size = BUFFER_SIZE;
    }

    return process_memory_->ReadMemory(address, scan_buffer_.data(), size);
}
