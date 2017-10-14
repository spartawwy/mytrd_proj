#include "winner_win.h"

#include "MySpinBox.h"
#include "HintList.h"

void WinnerWin::InitEqSectionTaskWin()
{
    // reset some widget ------------
#if 1
     
    ui.dbspbox_eqsec_raise_percent->setDecimals(1);
    ui.dbspbox_eqsec_raise_percent->setMaximum(100);
    ui.dbspbox_eqsec_raise_percent->setMinimum(0.1);

	ui.dbspbox_eqsec_fall_percent->setDecimals(1);
    ui.dbspbox_eqsec_fall_percent->setMaximum(100);
    ui.dbspbox_eqsec_fall_percent->setMinimum(0.1);

	auto obj_name = ui.spinBox_eqsec_quantity->objectName();
    const QRect geome_val = ui.spinBox_eqsec_quantity->geometry();
    delete ui.spinBox_eqsec_quantity;
    ui.spinBox_eqsec_quantity = new MySpinBox(ui.tab_eq_section_trd);
    ui.spinBox_eqsec_quantity->setSingleStep(100);
    ui.spinBox_eqsec_quantity->setObjectName(obj_name);
    ui.spinBox_eqsec_quantity->setGeometry(geome_val);
    ui.spinBox_eqsec_quantity->setMaximum(1000000000);
#endif
	ui.dbspbox_eqsec_max_price->setDisabled(true);
	ui.dbspbox_eqsec_min_price->setDisabled(true);
	ui.cb_max_stop_trigger->setChecked(false);
	 
	bool ret = QObject::connect(ui.le_eqsec_stock, SIGNAL(textChanged(QString)), this, SLOT(FlushFromStationListWidget(QString)));
    m_eqsec_list_hint_ = new HintList(this, ui.le_eqsec_stock);
    m_eqsec_list_hint_->hide();
    ret = QObject::connect(m_eqsec_list_hint_, SIGNAL(clicked(QModelIndex)), this, SLOT(OnClickedListWidget(QModelIndex)));
    ret = QObject::connect(m_eqsec_list_hint_, SIGNAL(choiceStr(QString)), this, SLOT(ChangeFromStationText(QString)));
     
	ret = QObject::connect(ui.cb_max_stop_trigger, SIGNAL(stateChanged(int)), SLOT(DoMaxStopTrigCheckBoxChanged(int)));
	ret = QObject::connect(ui.cb_min_clear_trigger, SIGNAL(stateChanged(int)), SLOT(DoMinClearTrigCheckBoxChanged(int)));

	ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("即时价"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_CUR)));
    ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("买一和卖一"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_BUYSELL_1)));
    ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("买二和卖二"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_BUYSELL_2)));
    ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("买三和卖三"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_BUYSELL_3)));
    ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("买四和卖四"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_BUYSELL_4)));
    ui.combox_eqsec_price_level->addItem(QString::fromLocal8Bit("买五和卖五"), QVariant(static_cast<int>(TypeQuoteLevel::PRICE_BUYSELL_5)));
	  
    ResetEqSectionTaskTime();

	ret = connect(ui.pbtn_add_eqsection_task, SIGNAL(clicked()), this, SLOT(DoAddEqSectionTask()));
}


void WinnerWin::DoAddEqSectionTask()
{

}

void WinnerWin::ResetEqSectionTaskTime()
{
	ui.timeEdit_eqsec_begin->setTime(QTime(9, 30, 0));
    ui.timeEdit_eqsec_end->setTime(QTime(15, 30, 0));
}

void WinnerWin::DoMaxStopTrigCheckBoxChanged(int stat)
{
	switch( stat )
	{
		case Qt::Unchecked:
			ui.dbspbox_eqsec_max_price->setDisabled(true);
			break;
		case Qt::Checked:
			ui.dbspbox_eqsec_max_price->setEnabled(true);
			break;
	}
}

void WinnerWin::DoMinClearTrigCheckBoxChanged(int stat)
{
	switch( stat )
	{
		case Qt::Unchecked:
			ui.dbspbox_eqsec_min_price->setDisabled(true);	
			break;
		case Qt::Checked:
			ui.dbspbox_eqsec_min_price->setEnabled(true);	
			break;
	}
}