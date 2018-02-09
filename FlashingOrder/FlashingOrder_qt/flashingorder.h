#ifndef FLASHINGORDER_H
#define FLASHINGORDER_H

#include <QtWidgets/QWidget>
#include "ui_flashingorder.h"

#include <unordered_map>
#include <string>
#include <mutex>

#include <QWaitCondition>
#include <QTimer>

#include "trade_proxy.h"
#include "mythread.h"

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

	bool GetWinTileAndStockName(QString& title, QString& stock_name);

	bool exit_flag() const { return exit_flag_; }

	void EmitKeySig(QString str) { emit key_sig(str); }

	void set_key_sig(bool val);

	QMutex & key_sig_mutex() { return key_sig_mutex_; }
	QWaitCondition & key_sig_wait_cond() { return key_sig_wait_cond_; }

private slots:

	//void DoNormalTimer();
	void DoKeySig(QString);

signals:

	void key_sig(QString);

private:
	Ui::FlashingOrderClass  ui;

	bool exit_flag_;

	std::string  target_win_title_tag_;

	volatile bool has_key_sig_; 
	std::mutex  key_sig_flag_mutex_;

	
	QMutex          key_sig_mutex_;
	QWaitCondition  key_sig_wait_cond_;

	MyThread  thread_;
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
