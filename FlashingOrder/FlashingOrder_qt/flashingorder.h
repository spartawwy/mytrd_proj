#ifndef FLASHINGORDER_H
#define FLASHINGORDER_H

#include <QtWidgets/QWidget>
#include "ui_flashingorder.h"

#include <QTimer>
#include <string>

class FlashingOrder : public QWidget
{
	Q_OBJECT

public:

	FlashingOrder(QWidget *parent = 0);
	~FlashingOrder();

	bool Init();

private slots:

	void DoNormalTimer();

private:
	Ui::FlashingOrderClass ui;

	std::string target_win_title_tag_;
	 
	QTimer  normal_timer_;
	int broker_id_;
	std::string account_no_;
	std::string account_pwd_;
};

#endif // FLASHINGORDER_H
