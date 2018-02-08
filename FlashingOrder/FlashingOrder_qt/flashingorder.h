#ifndef FLASHINGORDER_H
#define FLASHINGORDER_H

#include <QtWidgets/QWidget>
#include "ui_flashingorder.h"

#include <unordered_map>

#include <QTimer>
#include <string>

#include "trade_proxy.h"

class Ticker;
class FlashingOrder : public QWidget
{
	Q_OBJECT

public:

	FlashingOrder(QWidget *parent = 0);
	~FlashingOrder();

	bool Init(int argc, char *argv[]);

	void HandleOrder(bool is_buy, const std::string &stock_name);

	TradeProxy& trade_proxy(){ return trade_proxy_; }

	std::shared_ptr<Ticker>& ticker() { return  ticker_; }

	bool GetWinTileAndStockName(std::string& title, std::string& stock_name);
private slots:

	void DoNormalTimer();

private:
	Ui::FlashingOrderClass  ui;

	std::string  target_win_title_tag_;
	 
	QTimer  normal_timer_;
	/*/int broker_id_;
	std::string account_no_;
	std::string account_pwd_;*/
	// (name, code)
	TradeProxy  trade_proxy_;
	int  trade_client_id_;

	std::unordered_map<std::string, std::string>  stock_name2code_;
	
	T_BrokerInfo  broker_info_;
	T_UserAccountInfo  account_info_;

	std::shared_ptr<Ticker>  ticker_;

	friend class DBMoudle;
};

FlashingOrder& AppInstance();

#endif // FLASHINGORDER_H
