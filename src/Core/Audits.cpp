#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>

#include "../Interfaces/Audits.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

Audits::Audits()
{
    filename = "./resources/database/audits.json";
    fs::create_directories("./resources/database");
    if (!fs::exists(filename)) {
        std::ofstream file(filename);
        file << "[]";
        file.close();
    }
}

Audits::Audits(const std::string& fname) : filename(fname) {}

Audits::~Audits() {}

bool Audits::AddAuditLog(AuditLogDto auditLog) {
    try {
        int fd = open(filename.c_str(), O_RDWR);
        if (fd == -1) return false;
        
        flock(fd, LOCK_EX);
        auto logs = LoadFromFile();
        
        auditLog.AuditLogId = GetNextAuditLogId();
        logs.push_back(auditLog);
        SaveToFile(logs);
        
        flock(fd, LOCK_UN);
        close(fd);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<AuditLogDto> Audits::GetAllAuditLogs() {
    try {
        return LoadFromFile();
    } catch (...) {
        return std::vector<AuditLogDto>();
    }
}

int Audits::GetNextAuditLogId() const {
    auto audit = LoadFromFile();
    if (audit.empty()) return 1;
    
    return std::max_element(audit.begin(), audit.end(),
        [](const AuditLogDto& a, const AuditLogDto& b) { 
            return a.AuditLogId < b.AuditLogId; 
        })->AuditLogId + 1;
}

void Audits::SaveToFile(const std::vector<AuditLogDto>& auditLogs) const {
    json j = json::array();
    for (const auto& auditLog : auditLogs) {
        json auditJson;
        auditJson["AuditLogId"] = auditLog.AuditLogId;
        auditJson["Action"] = auditLog.Action;
        auditJson["ClientIp"] = auditLog.ClientIp;
        auditJson["DateCreated"] = auditLog.DateCreated;
        auditJson["Description"] = auditLog.Description;
        auditJson["MachineName"] = auditLog.MachineName;
        j.push_back(auditJson);
    }
    
    std::ofstream file(filename);
    file << std::setw(4) << j << std::endl;
}

std::vector<AuditLogDto> Audits::LoadFromFile() const {
    std::vector<AuditLogDto> auditLogs;
    std::ifstream file(filename);
    if (!file.is_open()) return auditLogs;
    
    json j;
    file >> j;
    
    for (const auto& auditJson : j) {
        AuditLogDto auditLog;
        auditLog.AuditLogId = auditJson["AuditLogId"];
        auditLog.Action = auditJson["Action"];
        auditLog.ClientIp = auditJson["ClientIp"];
        auditLog.DateCreated = auditJson["DateCreated"];
        auditLog.Description = auditJson["Description"];
        auditLog.MachineName = auditJson["MachineName"];
    
        auditLogs.push_back(auditLog);
    }
    
    return auditLogs;
}