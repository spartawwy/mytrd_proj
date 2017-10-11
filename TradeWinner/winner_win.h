#ifndef TRADE_WINNER_H
#define TRADE_WINNER_H

#include <memory>
#include <list>

#include <QtWidgets/QMainWindow>
#include "ui_trade_winner.h"

#include "common.h"

//class MyThread;
//class StockTicker;
class StrategyTask;
class WinnerApp;
class HintList;

class WinnerWin : public QMainWindow
{
    Q_OBJECT

public:

    WinnerWin(WinnerApp *app, QWidget *parent = 0);
    ~WinnerWin();

    void Init();

    virtual void keyPressEvent(QKeyEvent *event) override;

public slots:

    void DoQueryCapital();

    void SlotAppendLog(char *);

	void SlotTabChanged(int);
    void SlotTbvTasksContextMenu(QPoint p);

    void SlotTbvTasksActionStart(bool);
    void SlotTbvTasksActionStop(bool);
    void SlotTbvTasksActionDel(bool);
    void SlotTbvTasksActionDetail(bool);

    void RemoveByTaskId(int task_id);

    // sell task win--------------------
    void DoAlertPercentChanged(double val);
    void DoSellTypeChanged(const QString&);
    void ChangeTabSellAssistantImg(TypeTask type);

    void DoLeStockEditingFinished();
    void DoLeStockChanged(const QString &);

    
    void DoSpboxQuantityEditingFinished();
     
    void DoAddTask();
    void DoTaskStatChangeSignal(StrategyTask*, int);
    void DoQueryPosition();

    void InsertIntoTbvTasklist(QTableView *tbv , T_TaskInformation &task_info);

    void ResetSellTabTaskTime();

    //---------------buy task related---
    void DoBuyAlertPercentChanged(double);
	void DoBuyTypeChanged(const QString&);
    void DoAddBuyTask();
    void DoQueryQtyCanBuy();

	void ResetBuyTabTaskTime();
    //------------------
	
    void ChangeTabBuyAssistantImg(TypeTask type);

    int TbvTasksCurRowTaskId();
    void FlushFromStationListWidget(QString str);
    void OnClickedListWidget(QModelIndex index);
    void ChangeFromStationText(QString text);

    void AssignHintListAndLineEdit(HintList *& p_list, QLineEdit *&p_edit, QDoubleSpinBox *&p_dbspinbox);

  signals:

    //void SigRemoveTask(int task_id);

protected:

    virtual void closeEvent(QCloseEvent * event) override;
      
private:

    void InitSellTaskWin();
    void SetupSellTaskWin();
    void FillSellTaskWin(TypeTask type, T_TaskInformation& info);
   

    // buy task
    void InitBuyTaskWin();
    void FillBuyTaskWin(TypeTask type, T_TaskInformation& info);

    Ui::TradeWinnerClass ui;
    WinnerApp *app_;
    QMenu *tbv_tasks_popMenu_;

    HintList *m_list_hint_;
	bool is_open_hint_;

    // buy task related
    HintList *m_bt_list_hint_; 

    double pre_close_price_;
    double buytask_pre_close_price_;
};

#endif // TRADE_WINNER_H
