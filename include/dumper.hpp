#include "pattern_scanner.cpp"
#include "logger.hpp"
#include "process_memory.hpp"
#include "ue_types.hpp"
#include <string>
#include <map>
#include <json/json.h>

class SDKDumper {
public:
    SDKDumper();
    ~SDKDumper();

    bool Initialize(const std::string& process_name);
    bool DumpSDK(const std::string& output_dir);

private:
    ProcessMemory* process_memory_;
    PatternScanner* pattern_scanner_;
    UE4::GlobalPointers globals_;
    
    bool FindGlobals();
    bool DumpObjects();
    bool DumpOffsets(const std::string& output_file);
    bool DumpJSON(const std::string& output_file);
    
    std::map<std::string, uint64_t> object_offsets_;
    std::map<std::string, Json::Value> class_info_;
};
