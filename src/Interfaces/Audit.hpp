#include <vector>;


#include "Common.hpp";

class Audit
{

public:
    Audit();
    Audit(const std::string &filename);
    ~Audit();

    bool AddAuditLog(AuditLogDto auditLog);
    std::vector<AuditLogDto> GetAllAuditLogs();

};