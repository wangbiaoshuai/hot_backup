#ifndef HOT_BACKUP_CLIENT_H_
#define HOT_BACKUP_CLIENT_H_

#include <string>

class HotBackupClient
{
public:
    HotBackupClient(const std::string& local_ip, int local_port, const std::string& server_ip, int server_port);
    ~HotBackupClient();

    int Start();

private:
    int ParsePackage(const char* msg);
    int ChangeServer();

private:
    std::string local_ip_;
    int local_port_;
    std::string server_ip_;
    int server_port_;
    int timeout_count_;

};
#endif // HOT_BACKUP_CLIENT_H_
