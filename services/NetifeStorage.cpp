//
// Created by Administrator on 2023/5/12.
//

#include "NetifeStorage.h"
#include <filesystem>
#include "../lib/log/easylogging++.h"

#define ELPP_THREAD_SAFE

using namespace std;
namespace Netife {
    std::mutex NetifeStorage::mutex;
    std::atomic<NetifeStorage *> NetifeStorage::instance;
    sqlite3 *NetifeStorage::sqliteClient;

    NetifeStorage::~NetifeStorage() {
        if (sqliteClient) {
            sqlite3_close_v2(sqliteClient);
            sqliteClient = nullptr;
        }
    }

    NetifeStorage *NetifeStorage::Instance() {
        // 双 Check 锁
        if (instance == nullptr) {
            std::lock_guard<std::mutex> lock(mutex);
            if (instance == nullptr) {
                instance = new NetifeStorage();
            }
        }
        return instance;
    }

    void NetifeStorage::PushTransStream(std::string UUID, int requestType, int applicationType, std::string protocol,
                                        std::string dstIpAddr, std::string dstIpPort, std::string srcIpAddr,
                                        std::string srcIpPort, int isRawText, std::string UUIDSUB,
                                        std::string rawText, std::string pid, std::string processName) {
        std::string sql = "INSERT INTO rawText (UUID, requestType, applicationType, protocol, dstIpAddr, dstIpPort, "
                          "srcIpAddr, srcIpPort, isRawText, UUIDSUB, rawTexts, pid, processName) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?); ";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, UUID.c_str(), UUID.length(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, requestType);
        sqlite3_bind_int(stmt, 3, applicationType);
        sqlite3_bind_text(stmt, 4, protocol.c_str(), protocol.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, dstIpAddr.c_str(), dstIpAddr.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, dstIpPort.c_str(), dstIpPort.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, srcIpAddr.c_str(), srcIpAddr.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 8, srcIpPort.c_str(), srcIpPort.length(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 9, isRawText);
        sqlite3_bind_text(stmt, 10, UUIDSUB.c_str(), UUIDSUB.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 11, rawText.c_str(), rawText.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 12, pid.c_str(), pid.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 13, processName.c_str(), processName.length(), SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::optional<std::vector<std::string>> NetifeStorage::GetRawTextByUUID(std::string UUID, std::optional<std::string> UUID_Sub) {
        std::string sql = "SELECT rawTexts FROM rawText WHERE UUID = '" + UUID + "'";
        if (!UUID_Sub.has_value()){
            sql += " AND UUIDSUB = '" + UUID_Sub.value() + "'";
        }

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        vector<string> res;
        if (result == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string rawText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                res.push_back(rawText);
            }
        }
        else {
            return nullopt;
        }

        sqlite3_finalize(stmt);

        return { res };
    }

    NetifeStorage::NetifeStorage() {
        sqliteClient = nullptr;
        filesystem::path rootPath("data");
        rootPath /= "dataStorage.bin";
        std::string rootStringPath = rootPath.string();

        const char *path = rootStringPath.c_str();

        // 根据文件路径打开数据库连接。如果数据库不存在，则创建。
        int result = sqlite3_open_v2(path, &sqliteClient,
                                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                     SQLITE_OPEN_SHAREDCACHE,
                                     nullptr);

        if (result != SQLITE_OK) {
            CLOG(ERROR, "NetifeDataStorage") << "Cannot find or open NetifeDataStorage !!!";
        }

        //如果不存在那么就建表
        if (!TryCommitSqlStatement("CREATE TABLE IF NOT EXISTS rawText (UUID varchar(42), "
                                   "requestType integer, applicationType integer, protocol varchar(6), "
                                   "dstIpAddr varchar(16), dstIpPort varchar(6), "
                                   "srcIpAddr varchar(16), srcIpPort varchar(6), isRawText integer, "
                                   "UUIDSUB varchar(42), rawTexts Text, "
                                   "pid varchar(6), processName varchar(42))")) {
            CLOG(ERROR, "NetifeDataStorage") << "Cannot set up basic Table !!!";
        }
    }

    bool NetifeStorage::TryCommitSqlStatement(const string &sqlStat) {
        const char *sqlSentence = sqlStat.c_str();
        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sqlSentence, -1, &stmt, nullptr);

        if (result == SQLITE_OK) {
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            return true;
        } else {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    std::optional<std::vector<std::string>> NetifeStorage::GetRawTextByDst(std::string Pid, std::optional<std::string> ProcessName) {
        std::string sql = "SELECT rawTexts FROM rawText WHERE pid = '" + Pid + "'";
        if (!ProcessName.has_value()){
            sql += " AND processName = '" + ProcessName.value() + "'";
        }

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        vector<string> res;
        if (result == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string rawText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                res.push_back(rawText);
            }
        }
        else {
            sqlite3_finalize(stmt);
            return nullopt;
        }

        sqlite3_finalize(stmt);

        return { res };
    }

    std::optional<std::vector<std::string>> NetifeStorage::GetRawTextByPid(std::string DstHost, std::optional<std::string> DstPort) {
        std::string sql = "SELECT rawTexts FROM rawText WHERE dstIpAddr = '" + DstHost + "'";
        if (!DstPort.has_value()){
            sql += " AND dstIpPort = '" + DstPort.value() + "'";
        }

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        vector<string> res;
        if (result == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string rawText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                res.push_back(rawText);
            }
        }
        else {
            sqlite3_finalize(stmt);
            return nullopt;
        }

        sqlite3_finalize(stmt);

        return { res };
    }

    std::optional<std::vector<std::string>> NetifeStorage::GetRawTextBySrc(std::string SrcHost, std::optional<std::string> SrcPort) {
        std::string sql = "SELECT rawTexts FROM rawText WHERE srcIpAddr = '" + SrcHost + "'";
        if (!SrcPort.has_value()){
            sql += " AND srcIpPort = '" + SrcPort.value() + "'";
        }

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        vector<string> res;
        if (result == SQLITE_OK) {

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                string rawText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                res.push_back(rawText);
            }
        }
        else {
            sqlite3_finalize(stmt);
            return nullopt;
        }

        sqlite3_finalize(stmt);

        return { res };
    }

    void NetifeStorage::AddSettings(std::string pluginsName, std::string className, std::string key, std::string value) {
        std::string sql = "INSERT INTO `" + pluginsName + "::" + className + "` (key, value) "
                          "VALUES (?, ?);";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), key.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, value.c_str(), value.length(), SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void NetifeStorage::UpdateSettings(std::string pluginsName, std::string className, std::string key, std::string value) {
        std::string sql = "UPDATE `" + pluginsName + "::" + className + "` SET value = ? WHERE key = ?";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, value.c_str(), value.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, key.c_str(), key.length(), SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void NetifeStorage::RemoveSettings(std::string pluginsName, std::string className, std::string key) {
        std::string sql = "DELETE FROM `" + pluginsName + "::" + className + "` WHERE key = ?";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, key.c_str(), key.length(), SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    bool NetifeStorage::ExitsSettings(std::string pluginsName, std::string className, std::string key) {
        std::string sql = "SELECT value FROM `" + pluginsName + "::" + className + "` WHERE key = " + key;

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        std::string res;
        if (result == SQLITE_OK) {

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                sqlite3_finalize(stmt);
                return true;
            }else{
                sqlite3_finalize(stmt);
                return false;
            }
        }
        else {
            sqlite3_finalize(stmt);
            return false;
        }
    }

    std::optional<std::string> NetifeStorage::GetSettings(std::string pluginsName, std::string className, std::string key) {
        std::string sql = "SELECT value FROM `" + pluginsName + "::" + className + "` WHERE key = " + key;

        sqlite3_stmt *stmt = nullptr;

        int result = sqlite3_prepare_v2(sqliteClient, sql.c_str(), -1, &stmt, nullptr);

        std::string res;
        if (result == SQLITE_OK) {

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                res = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            }
        }
        else {
            sqlite3_finalize(stmt);
            return nullopt;
        }

        sqlite3_finalize(stmt);

        return { res };
    }

    void NetifeStorage::BuildDataTable(std::string pluginsName, std::string className) {
        TryCommitSqlStatement("CREATE TABLE IF NOT EXISTS `" + pluginsName + "::" + className + "` (key varchar(256), "
                              "value varchar(256))");
    }
} // Netife