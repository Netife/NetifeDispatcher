//
// Created by Administrator on 2023/5/12.
//

#ifndef NETIFEDISPATCHER_NETIFESTORAGE_H
#define NETIFEDISPATCHER_NETIFESTORAGE_H
#include <sqlite3.h>
#include <iostream>
#include <mutex>
#include <map>
#include <vector>
#include <optional>

namespace Netife {

    class NetifeStorage {
    private:
        NetifeStorage();
        static std::atomic<NetifeStorage*> instance;
        static std::mutex mutex;
        static sqlite3 *sqliteClient;
        bool TryCommitSqlStatement(const std::string& sqlStat);
    public:
        //单例
        NetifeStorage(const NetifeStorage&) = delete;
        NetifeStorage& operator=(const NetifeStorage&) = delete;
        ~NetifeStorage();
        static NetifeStorage *Instance();
        void PushTransStream(std::string UUID, int requestType, int applicationType, std::string protocol,
                                            std::string dstIpAddr, std::string dstIpPort, std::string srcIpAddr,
                                            std::string srcIpPort, int isRawText, std::string UUID_SUB,
                                            std::string rawText, std::string pid, std::string processName);
        std::optional<std::vector<std::string>> GetRawTextByUUID(std::string UUID, std::optional<std::string> UUID_Sub);
        std::optional<std::vector<std::string>> GetRawTextByPid(std::string Pid, std::optional<std::string> ProcessName);
        std::optional<std::vector<std::string>> GetRawTextByDst(std::string DstHost, std::optional<std::string> DstPort);
        std::optional<std::vector<std::string>> GetRawTextBySrc(std::string SrcHost, std::optional<std::string> SrcPort);
        void BuildDataTable(std::string pluginsName, std::string className);
        void AddSettings(std::string pluginsName, std::string className, std::string key, std::string value);
        void UpdateSettings(std::string pluginsName, std::string className, std::string key, std::string value);
        void RemoveSettings(std::string pluginsName, std::string className, std::string key);
        bool ExitsSettings(std::string pluginsName, std::string className, std::string key);
        std::optional<std::string> GetSettings(std::string pluginsName, std::string className, std::string key);
    };

} // Netife

#endif //NETIFEDISPATCHER_NETIFESTORAGE_H
