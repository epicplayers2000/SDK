#include "process_memory.hpp"
#include "logger.hpp"
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <dirent.h>

ProcessMemory::ProcessMemory(const std::string& process_name)
    : process_name_(process_name), process_id_(0), process_handle_(-1) {}

ProcessMemory::~ProcessMemory() {
    DetachProcess();
}

bool ProcessMemory::AttachProcess() {
    if (!FindProcessId()) {
        LOG_ERROR("Failed to find process: " + process_name_);
        return false;
    }

    // Open /proc/pid/mem for reading
    std::string mem_path = "/proc/" + std::to_string(process_id_) + "/mem";
    process_handle_ = open(mem_path.c_str(), O_RDONLY);
    
    if (process_handle_ < 0) {
        LOG_ERROR("Failed to open " + mem_path);
        return false;
    }

    LOG_INFO("Attached to process: " + process_name_ + " (PID: " + std::to_string(process_id_) + ")");
    return true;
}

bool ProcessMemory::DetachProcess() {
    if (process_handle_ >= 0) {
        close(process_handle_);
        process_handle_ = -1;
        LOG_INFO("Detached from process");
        return true;
    }
    return false;
}

bool ProcessMemory::ReadMemory(uint64_t address, void* buffer, size_t size) {
    if (process_handle_ < 0) {
        LOG_ERROR("Process not attached");
        return false;
    }

    if (lseek(process_handle_, address, SEEK_SET) < 0) {
        return false;
    }

    ssize_t bytes_read = read(process_handle_, buffer, size);
    return bytes_read == (ssize_t)size;
}

bool ProcessMemory::ReadString(uint64_t address, std::string& out_str, size_t max_length) {
    std::vector<char> buffer(max_length);
    if (!ReadMemory(address, buffer.data(), max_length)) {
        return false;
    }

    out_str = std::string(buffer.data());
    return true;
}

bool ProcessMemory::ReadWString(uint64_t address, std::wstring& out_str, size_t max_length) {
    std::vector<wchar_t> buffer(max_length);
    if (!ReadMemory(address, buffer.data(), max_length * sizeof(wchar_t))) {
        return false;
    }

    out_str = std::wstring(buffer.data());
    return true;
}

bool ProcessMemory::WriteMemory(uint64_t address, const void* buffer, size_t size) {
    if (process_handle_ < 0) {
        LOG_ERROR("Process not attached");
        return false;
    }

    // Open process memory for writing
    std::string mem_path = "/proc/" + std::to_string(process_id_) + "/mem";
    int write_handle = open(mem_path.c_str(), O_WRONLY);
    if (write_handle < 0) {
        LOG_WARNING("Cannot write to process memory without proper permissions");
        return false;
    }

    if (lseek(write_handle, address, SEEK_SET) < 0) {
        close(write_handle);
        return false;
    }

    ssize_t bytes_written = write(write_handle, buffer, size);
    close(write_handle);
    return bytes_written == (ssize_t)size;
}

uint64_t ProcessMemory::GetModuleBase(const std::string& module_name) {
    std::string maps_path = "/proc/" + std::to_string(process_id_) + "/maps";
    std::ifstream maps_file(maps_path);
    
    if (!maps_file.is_open()) {
        LOG_ERROR("Cannot open " + maps_path);
        return 0;
    }

    std::string line;
    while (std::getline(maps_file, line)) {
        if (line.find(module_name) != std::string::npos) {
            std::stringstream ss(line);
            std::string addr_range;
            ss >> addr_range;
            
            size_t dash_pos = addr_range.find('-');
            if (dash_pos != std::string::npos) {
                std::string base_str = addr_range.substr(0, dash_pos);
                return std::stoull(base_str, nullptr, 16);
            }
        }
    }

    LOG_WARNING("Module not found: " + module_name);
    return 0;
}

std::vector<std::pair<std::string, uint64_t>> ProcessMemory::GetModules() {
    std::vector<std::pair<std::string, uint64_t>> modules;
    std::string maps_path = "/proc/" + std::to_string(process_id_) + "/maps";
    std::ifstream maps_file(maps_path);
    
    if (!maps_file.is_open()) {
        return modules;
    }

    std::string line;
    while (std::getline(maps_file, line)) {
        // Format: address perms offset dev inode pathname
        std::stringstream ss(line);
        std::string addr_range, perms, offset, dev, inode, pathname;
        
        ss >> addr_range >> perms >> offset >> dev >> inode >> pathname;
        
        if (!pathname.empty() && pathname[0] == '/') {
            size_t dash_pos = addr_range.find('-');
            if (dash_pos != std::string::npos) {
                std::string base_str = addr_range.substr(0, dash_pos);
                uint64_t base = std::stoull(base_str, nullptr, 16);
                modules.push_back({pathname, base});
            }
        }
    }

    return modules;
}

bool ProcessMemory::GetMemoryMap(std::vector<std::pair<uint64_t, uint64_t>>& regions) {
    std::string maps_path = "/proc/" + std::to_string(process_id_) + "/maps";
    std::ifstream maps_file(maps_path);
    
    if (!maps_file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(maps_file, line)) {
        std::stringstream ss(line);
        std::string addr_range;
        ss >> addr_range;
        
        size_t dash_pos = addr_range.find('-');
        if (dash_pos != std::string::npos) {
            std::string start_str = addr_range.substr(0, dash_pos);
            std::string end_str = addr_range.substr(dash_pos + 1);
            
            uint64_t start = std::stoull(start_str, nullptr, 16);
            uint64_t end = std::stoull(end_str, nullptr, 16);
            regions.push_back({start, end});
        }
    }

    return !regions.empty();
}

bool ProcessMemory::FindProcessId() {
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            std::string cmdline_path = std::string("/proc/") + entry->d_name + "/cmdline";
            std::ifstream cmdline_file(cmdline_path);
            
            if (cmdline_file.is_open()) {
                std::string cmdline;
                std::getline(cmdline_file, cmdline);
                
                if (cmdline.find(process_name_) != std::string::npos) {
                    process_id_ = std::stoul(entry->d_name);
                    closedir(proc_dir);
                    return true;
                }
            }
        }
    }

    closedir(proc_dir);
    return false;
}
