#ifndef FLASHINGORDER_H
#define FLASHINGORDER_H

#include <QtWidgets/QWidget>
#include "ui_flashingorder.h"

#include <QTimer>

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

	QTimer  normal_timer_;
};

#endif // FLASHINGORDER_H
