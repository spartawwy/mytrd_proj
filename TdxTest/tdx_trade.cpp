#include <string>


bool Login()
{

int nQsid = 32;
std::string sHost = "180.153.18.180";
int nPort = 7708;
std::string sVersion = "8.27";
int nBranchID = 1;
char nAccountType = 8;
std::string sAccountNo = "880003767427";
std::string sTradeAccountNo = "880003767427";
//std::string sPassword = "123321"; 
std::string sTxPassword = "";
trade_client_id_ = trade_delegater_->Logon(nQsid, /*ip*/sHost.c_str(), nPort, sVersion.c_str(), nBranchID, nAccountType, account_no
                                           , trade_account, trade_pwd, txpwd, error_info);

}
