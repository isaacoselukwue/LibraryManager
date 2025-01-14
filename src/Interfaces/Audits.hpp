#ifndef AUDITS_HPP
#define AUDITS_HPP

#include <vector>
#include "Common.hpp"

struct AuditLogDto
{
    int AuditLogId;
    std::string ClientIp;
    std::string MachineName;
    std::string Action;
    std::string Description;
    std::time_t DateCreated;
};

class Audits
{
private:
    std::string filename;
    std::vector<AuditLogDto> LoadFromFile() const;
    void SaveToFile(const std::vector<AuditLogDto>& auditLogs) const;
    int GetNextAuditLogId() const;

public:
    Audits();
    Audits(const std::string &filename);
    ~Audits();

    bool AddAuditLog(AuditLogDto auditLog);
    std::vector<AuditLogDto> GetAllAuditLogs();

};

#endif