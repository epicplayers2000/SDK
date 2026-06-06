#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "dumper.hpp"
#include "logger.hpp"

int main() {
    // Initialize logger
    std::string log_file = "/sdcard/UE4Dumper/dumper.log";
    Logger::Initialize(log_file);

    LOG_INFO("========================================");
    LOG_INFO("PUBG Mobile UE4 SDK Dumper v1.0");
    LOG_INFO("ARM64 - External Memory Reading");
    LOG_INFO("========================================");

    // Check if running as root
    if (geteuid() != 0) {
        LOG_ERROR("This tool must be run as root!");
        LOG_ERROR("Use: su -c /data/local/tmp/ue4dumper");
        return 1;
    }

    LOG_INFO("Running as root - OK");

    // Create output directory
    std::string output_dir = "/sdcard/UE4Dumper";
    if (mkdir(output_dir.c_str(), 0755) && errno != EEXIST) {
        LOG_ERROR("Failed to create output directory: " + output_dir);
        return 1;
    }
    LOG_INFO("Output directory: " + output_dir);

    // Initialize dumper
    SDKDumper dumper;
    
    LOG_INFO("Waiting for PUBG Mobile to load...");
    sleep(2);

    if (!dumper.Initialize("com.tencent.ig")) {
        LOG_ERROR("Failed to initialize dumper");
        return 1;
    }

    // Perform dump
    if (!dumper.DumpSDK(output_dir)) {
        LOG_ERROR("SDK dump failed");
        return 1;
    }

    LOG_INFO("========================================");
    LOG_INFO("SDK dump completed successfully!");
    LOG_INFO("Output location: " + output_dir);
    LOG_INFO("========================================");

    Logger::Shutdown();
    return 0;
}
