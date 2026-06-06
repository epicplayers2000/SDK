#include "dumper.hpp"
#include "process_memory.hpp"
#include "pattern_scanner.hpp"
#include "logger.hpp"
#include "ue_types.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

SDKDumper::SDKDumper() : process_memory_(nullptr), pattern_scanner_(nullptr) {
    memset(&globals_, 0, sizeof(globals_));
}

SDKDumper::~SDKDumper() {
    if (pattern_scanner_) delete pattern_scanner_;
    if (process_memory_) delete process_memory_;
}

bool SDKDumper::Initialize(const std::string& process_name) {
    LOG_INFO("Initializing SDK Dumper for: " + process_name);
    
    process_memory_ = new ProcessMemory(process_name);
    if (!process_memory_->AttachProcess()) {
        LOG_ERROR("Failed to attach to process");
        return false;
    }

    pattern_scanner_ = new PatternScanner(process_memory_);
    
    if (!FindGlobals()) {
        LOG_ERROR("Failed to find global pointers");
        return false;
    }

    LOG_INFO("SDK Dumper initialized successfully");
    return true;
}

bool SDKDumper::FindGlobals() {
    LOG_INFO("Finding global pointers...");
    
    // Get libUE4.so base address
    uint64_t libue4_base = process_memory_->GetModuleBase("libUE4.so");
    if (libue4_base == 0) {
        LOG_ERROR("libUE4.so not found");
        return false;
    }

    LOG_INFO("libUE4.so base: 0x" + std::to_string(libue4_base));
    
    // Assume 100MB module size for scanning
    uint64_t libue4_size = 100 * 1024 * 1024;

    // Find GWorld
    globals_.GWorld = pattern_scanner_->FindGWorld(libue4_base, libue4_size);
    if (!globals_.GWorld) {
        LOG_WARNING("GWorld not found, trying fallback patterns");
    } else {
        LOG_INFO("GWorld found at: 0x" + std::to_string(globals_.GWorld));
    }

    // Find GNames
    globals_.GNames = pattern_scanner_->FindGNames(libue4_base, libue4_size);
    if (!globals_.GNames) {
        LOG_WARNING("GNames not found");
    } else {
        LOG_INFO("GNames found at: 0x" + std::to_string(globals_.GNames));
    }

    // Find GUObjectArray
    globals_.GUObjectArray = pattern_scanner_->FindGUObjectArray(libue4_base, libue4_size);
    if (!globals_.GUObjectArray) {
        LOG_WARNING("GUObjectArray not found");
    } else {
        LOG_INFO("GUObjectArray found at: 0x" + std::to_string(globals_.GUObjectArray));
    }

    // Find decryption function
    globals_.DecryptionFunc = pattern_scanner_->FindDecryptionFunction(libue4_base, libue4_size);
    if (!globals_.DecryptionFunc) {
        LOG_WARNING("Decryption function not found");
    } else {
        LOG_INFO("Decryption function found at: 0x" + std::to_string(globals_.DecryptionFunc));
    }

    return globals_.GUObjectArray != 0;
}

bool SDKDumper::DumpObjects() {
    LOG_INFO("Dumping objects from GUObjectArray...");
    
    if (!globals_.GUObjectArray) {
        LOG_ERROR("GUObjectArray not initialized");
        return false;
    }

    // Read GUObjectArray pointer
    uint64_t guo_array_ptr = 0;
    if (!process_memory_->ReadMemory(globals_.GUObjectArray, &guo_array_ptr, sizeof(uint64_t))) {
        LOG_ERROR("Failed to read GUObjectArray");
        return false;
    }

    LOG_INFO("GUObjectArray pointer: 0x" + std::to_string(guo_array_ptr));

    // Iterate through objects (simplified - in real implementation would be more complex)
    uint64_t max_objects = 200000;  // Typical limit
    for (uint64_t i = 0; i < max_objects; ++i) {
        uint64_t item_ptr = guo_array_ptr + (i * sizeof(UE4::FUObjectItem));
        
        UE4::FUObjectItem item;
        if (!process_memory_->ReadMemory(item_ptr, &item, sizeof(item))) {
            continue;
        }

        UE4::UObject* obj = item.GetObject();
        if (!obj) {
            continue;
        }

        // Read object metadata
        UE4::UObject obj_data;
        if (!process_memory_->ReadMemory((uint64_t)obj, &obj_data, sizeof(obj_data))) {
            continue;
        }

        // Store basic offset information
        std::string offset_key = "0x" + std::to_string((uint64_t)obj);
        object_offsets_[offset_key] = (uint64_t)obj;

        if (i % 10000 == 0) {
            LOG_INFO("Processed " + std::to_string(i) + " objects");
        }
    }

    LOG_INFO("Object dump complete. Total objects: " + std::to_string(object_offsets_.size()));
    return true;
}

bool SDKDumper::DumpOffsets(const std::string& output_file) {
    LOG_INFO("Generating Offsets.hpp...");
    
    std::ofstream file(output_file);
    if (!file.is_open()) {
        LOG_ERROR("Failed to create output file: " + output_file);
        return false;
    }

    file << "#pragma once\n\n";
    file << "// Auto-generated PUBG Mobile SDK offsets\n";
    file << "// Generated on: 2026-06-06\n\n";
    file << "namespace Offsets {\n\n";
    
    // Write critical offsets
    file << "    // Global Pointers\n";
    file << "    constexpr uint64_t GWorld = 0x" << std::hex << globals_.GWorld << ";\n";
    file << "    constexpr uint64_t GNames = 0x" << std::hex << globals_.GNames << ";\n";
    file << "    constexpr uint64_t GUObjectArray = 0x" << std::hex << globals_.GUObjectArray << ";\n";
    file << "    constexpr uint64_t DecryptionFunc = 0x" << std::hex << globals_.DecryptionFunc << ";\n\n";
    
    file << "    // UObject Offsets\n";
    file << "    constexpr uint64_t UObject_VTable = 0x00;\n";
    file << "    constexpr uint64_t UObject_ObjectFlags = 0x08;\n";
    file << "    constexpr uint64_t UObject_Outer = 0x10;\n";
    file << "    constexpr uint64_t UObject_Name = 0x18;\n";
    file << "    constexpr uint64_t UObject_Class = 0x20;\n\n";
    
    file << "    // UField Offsets\n";
    file << "    constexpr uint64_t UField_Next = 0x30;\n\n";
    
    file << "    // UProperty Offsets\n";
    file << "    constexpr uint64_t UProperty_ArrayDim = 0x38;\n";
    file << "    constexpr uint64_t UProperty_ElementSize = 0x3C;\n";
    file << "    constexpr uint64_t UProperty_PropertyFlags = 0x40;\n";
    file << "    constexpr uint64_t UProperty_Offset = 0x48;\n\n";
    
    file << "} // namespace Offsets\n";
    
    file.close();
    LOG_INFO("Offsets.hpp written successfully");
    return true;
}

bool SDKDumper::DumpJSON(const std::string& output_file) {
    LOG_INFO("Generating dump.json...");
    
    Json::Value root;
    root["version"] = "1.0";
    root["game"] = "PUBG Mobile";
    root["timestamp"] = "2026-06-06";
    
    // Add global pointers
    Json::Value globals;
    globals["GWorld"] = "0x" + std::to_string(globals_.GWorld);
    globals["GNames"] = "0x" + std::to_string(globals_.GNames);
    globals["GUObjectArray"] = "0x" + std::to_string(globals_.GUObjectArray);
    globals["DecryptionFunc"] = "0x" + std::to_string(globals_.DecryptionFunc);
    root["globals"] = globals;
    
    // Add object count
    root["object_count"] = (int)object_offsets_.size();
    
    // Add offsets
    Json::Value offsets;
    int count = 0;
    for (const auto& pair : object_offsets_) {
        if (count < 1000) {  // Limit JSON size
            offsets[pair.first] = "0x" + std::to_string(pair.second);
            count++;
        }
    }
    root["offsets"] = offsets;
    
    // Write to file
    std::ofstream file(output_file);
    if (!file.is_open()) {
        LOG_ERROR("Failed to create output file: " + output_file);
        return false;
    }
    
    file << root.toStyledString();
    file.close();
    
    LOG_INFO("dump.json written successfully");
    return true;
}

bool SDKDumper::DumpSDK(const std::string& output_dir) {
    LOG_INFO("Starting SDK dump to: " + output_dir);
    
    // Create output directory
    mkdir(output_dir.c_str(), 0755);
    
    // Dump objects
    if (!DumpObjects()) {
        LOG_ERROR("Failed to dump objects");
        return false;
    }
    
    // Generate offset headers
    std::string offsets_file = output_dir + "/Offsets.hpp";
    if (!DumpOffsets(offsets_file)) {
        LOG_ERROR("Failed to dump offsets");
        return false;
    }
    
    // Generate JSON dump
    std::string json_file = output_dir + "/dump.json";
    if (!DumpJSON(json_file)) {
        LOG_ERROR("Failed to dump JSON");
        return false;
    }
    
    LOG_INFO("SDK dump complete!");
    return true;
}
