#ifndef HOT_BACKUP_SERVER_H_
#define HOT_BACKUP_SERVER_H_

#include <string>

class HotBackupServer
{
public:
    HotBackupServer(const std::string& ip, int port);
    ~HotBackupServer();

    void Start();
    void HeartBeat();

private:
    int StartHeartBeat();
    int StopHeartBeat();
    int SwitchIp();

private:
    int CheckDatabaseStatus();
    int CheckMonitorStatus();
    int RestartDatabase();
    int RestartMonitor();

private:
    std::string server_ip_;
    int server_port_;
    int abnormal_count_;
    volatile bool flag_switch_;
    pthread_t heart_beat_thread_;
    volatile bool stop_heart_beat_;
};
#endif // HOT_BACKUP_SERVER_H_
