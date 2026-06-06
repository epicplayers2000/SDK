#pragma once

#include <string>
#include <vector>
#include <cstdint>

class ProcessMemory {
public:
    ProcessMemory(const std::string& process_name);
    ~ProcessMemory();

    // Process management
    bool AttachProcess();
    bool DetachProcess();
    bool IsAttached() const { return process_handle_ >= 0; }
    uint32_t GetProcessId() const { return process_id_; }

    // Memory reading
    bool ReadMemory(uint64_t address, void* buffer, size_t size);
    bool ReadString(uint64_t address, std::string& out_str, size_t max_length = 256);
    bool ReadWString(uint64_t address, std::wstring& out_str, size_t max_length = 256);

    // Memory writing (use with caution)
    bool WriteMemory(uint64_t address, const void* buffer, size_t size);

    // Get module base
    uint64_t GetModuleBase(const std::string& module_name);

    // Get all module information
    std::vector<std::pair<std::string, uint64_t>> GetModules();

    // Get memory map
    bool GetMemoryMap(std::vector<std::pair<uint64_t, uint64_t>>& regions);

private:
    std::string process_name_;
    uint32_t process_id_;
    int process_handle_;

    bool FindProcessId();
};
