#ifndef DBMOULDE_H_DSFWDSF_
#define DBMOULDE_H_DSFWDSF_

#include <string>
#include <atomic>
#include <unordered_map>

#include "common.h"

namespace SQLite
{
    class SQLiteConnection;
}

//namespace TSystem
//{
//    class LocalLogger;
//}
class WinnerApp;
class DBMoudle
{
public:

    DBMoudle(WinnerApp *app);
    ~DBMoudle();

    void Init();

    void LoadAllUserBrokerInfo();
    void LoadAllTaskInfo(std::unordered_map<int, std::shared_ptr<T_TaskInformation> > &taskinfos);

    T_UserAccountInfo * FindUserAccountInfo(int user_id);
    T_BrokerInfo * FindUserBrokerByUser(int user_id);
    T_BrokerInfo * FindUserBrokerByBroker(int id);
    std::shared_ptr<T_UserAccountInfo> FindAccountInfoByAccNoAndBrokerId(const std::string& accno, int broker_id);
    std::vector<T_BrokerInfo> GetAllBrokerInfo();
     

    bool SaveUserinformation(T_UserInformation &info);
    bool AddAccountInfo(T_AccountInformation &info);
    bool UpdateAccountInfo(T_AccountInformation &info);
    int FindBorkerIdByAccountID(int account_id);

    bool AddTaskInfo(std::shared_ptr<T_TaskInformation> &info);
    bool DelTaskInfo(int task_id);
    bool UpdateTaskInfo(T_TaskInformation &info);

    bool AddHisTask(std::shared_ptr<T_TaskInformation>& info);

    bool IsTaskExists(int user_id, TypeTask type, const std::string& stock);

    // ret: > 0 -- userid, else fail
    int CheckLogin(const std::string& name, const std::string& pwd, T_UserInformation *user_info=nullptr);
     
    void GetStockCode(const std::string &code, std::vector<T_StockCodeName>& ret);
    std::string GetStockName(const std::string &code);

private:

    DBMoudle(DBMoudle&);
    DBMoudle& operator = (DBMoudle&);
     
    //TSystem::LocalLogger *local_logger_;
    WinnerApp *app_;
    std::shared_ptr<SQLite::SQLiteConnection>  db_conn_;

    void Open(std::shared_ptr<SQLite::SQLiteConnection>& db_conn);

    // (broker id, broker info)
    std::unordered_map<int, T_BrokerInfo>  broker_info_map_;
    // (user id, account info) 
    std::unordered_map<int, T_UserAccountInfo> user_account_info_map_;

    int max_accoun_id_;
	//std::atomic_uint64_t max_task_id_;
};

#endif