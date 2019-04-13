#include "winner_win.h"

#include <qdebug.h>
#include <qmessagebox.h> 
#include <QtGui/QStandardItemModel>
#include <qtextcodec.h>

#include <TLib/core/tsystem_utility_functions.h>
#include <TLib/core/tsystem_time.h>

//#include "mythread.h"
#include "winner_app.h"

#include "strategy_task.h"
#include "breakdown_task.h"

#include "stock_ticker.h"
#include "MySpinBox.h"
#include "HintList.h"
#include "calc_win.h"

//static const QString cst_str_breakout_buy = QString::fromLocal8Bit("ͻ������");
//static const QString cst_str_Inflection_buy = QString::fromLocal8Bit("�յ�����");

static const int cst_tasklist_tab_index  = 0;
static const int cst_buytask_tab_index   = 1;
static const int cst_selltask_tab_index  = 2;
static const int cst_eqtask_tab_index    = 3;
static const int cst_adveqtask_tab_index = 4;
static const int cst_capital_tab_index   = 5;
static const int cst_log_tab_index       = 6;
static const int cst_indextask_tab_index = 7;

static const unsigned short cst_col_count = 11;

static const int cst_tbview_tasks_rowindex_task_id = 0;
static const int cst_tbview_tasks_rowindex_state = 1;
static const int cst_tbview_tasks_rowindex_task_name = 2;
static const int cst_tbview_tasks_rowindex_stock_name = 3;
static const int cst_tbview_tasks_rowindex_cur_price = 4;
static const int cst_tbview_tasks_rowindex_trigger_price = 5;
static const int cst_tbview_tasks_rowindex_quantity = 6;
static const int cst_tbview_tasks_rowindex_price_level = 7;
static const int cst_tbview_tasks_rowindex_start_time = 8;
static const int cst_tbview_tasks_rowindex_end_time = 9;
static const int cst_tbview_tasks_rowindex_task_type = 10;
static const int cst_tbview_tasks_rowindex_pretrigged = 11;

static const int cst_tab_index_task_list = 0;
static const int cst_tab_index_buy_task = 1;
static const int cst_tab_index_sell_task = 2;
static const int cst_tab_index_eqsec_task = 3;
static const int cst_tab_index_adveq_task = 4;
static const int cst_tab_capital = 5;
static const int cst_tab_index_log = 6;
static const int cst_tab_index_stkindex_task = 7;

static const int cst_small_width = 60;
static const int cst_normal_width = 80;
static const int cst_big_width = 120;

WinnerWin::WinnerWin(WinnerApp *app, QWidget *parent)
    : QMainWindow(parent)
    , app_(app)
    , tbv_tasks_popMenu_(nullptr)
    , m_list_hint_(nullptr)
	, is_open_hint_(true)
    , m_bt_list_hint_(nullptr)
	, m_eqsec_list_hint_(nullptr)
	, m_indtrd_list_hint_(nullptr)
    , cur_price_(0.0)
    , buytask_cur_price_(0.0)
	, eqsec_task_cur_price_(0.0)
    , status_label_(nullptr)
    , calc_win_(nullptr)
    , flash_win_timer_(nullptr)
{
    ui.setupUi(this);
	//-------------------------
	//this->setWindowTitle("my trade");
	 
#if 1
    //------------------tab task list 
    QStandardItemModel * model = new QStandardItemModel(0, cst_col_count, this);
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_task_id, new QStandardItem(QString::fromLocal8Bit("�����")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_task_id)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_state, new QStandardItem(QString::fromLocal8Bit("����״̬")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_state)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_task_name, new QStandardItem(QString::fromLocal8Bit("��������")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_task_name)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_stock_name, new QStandardItem(QString::fromLocal8Bit("��Ʊ��ָ��")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_stock_name)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_cur_price, new QStandardItem(QString::fromLocal8Bit("��ǰ�۸�")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_cur_price)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_trigger_price, new QStandardItem(QString::fromLocal8Bit("�����۸�")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_trigger_price)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_quantity, new QStandardItem(QString::fromLocal8Bit("��������")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_quantity)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_price_level, new QStandardItem(QString::fromLocal8Bit("����ʹ�ü۸�")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_price_level)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_start_time, new QStandardItem(QString::fromLocal8Bit("��ʼʱ��")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_start_time)->setTextAlignment(Qt::AlignCenter);
    
    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_end_time, new QStandardItem(QString::fromLocal8Bit("����ʱ��")));
    model->horizontalHeaderItem(cst_tbview_tasks_rowindex_end_time)->setTextAlignment(Qt::AlignCenter);

    model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_task_type, new QStandardItem(QString::fromLocal8Bit("����")));
    
	model->setHorizontalHeaderItem(cst_tbview_tasks_rowindex_pretrigged, new QStandardItem(QString::fromLocal8Bit("�ϴδ�����")));
	model->horizontalHeaderItem(cst_tbview_tasks_rowindex_pretrigged)->setTextAlignment(Qt::AlignCenter);

    ui.tbview_tasks->setModel(model);
    ui.tbview_tasks->setColumnWidth(cst_tbview_tasks_rowindex_task_type, 1);
    ui.tbview_tasks->setColumnWidth(cst_tbview_tasks_rowindex_pretrigged, 70);
    ui.tbview_tasks->setColumnWidth(cst_tbview_tasks_rowindex_stock_name, cst_big_width);
    ui.tbview_tasks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bool ret = connect(ui.tbview_tasks, SIGNAL(doubleClicked(const QModelIndex)), this, SLOT(DoTabTasksDbClick(const QModelIndex)));
    //------------------tab add sell task
    SetupSellTaskWin();
    InitSellTaskWin();

#endif
   
    //------------------tab add buy task------------ 
    InitBuyTaskWin();

    //------------------tab equal section task----------------
	InitEqSectionTaskWin();
	 
    //------------------tab adveq task----------------
	InitAdveqTaskWin();

    //------------------tab index trade task----------------
	InitIndexTradeWin();

	connect(ui.pbtn_query_capital, SIGNAL(clicked()), this, SLOT(DoQueryCapital()));

	status_label_ = new QLabel("");
    status_label_->setIndent(3);
    statusBar()->addWidget(status_label_, 1);
	statusBar()->update();

}

WinnerWin::~WinnerWin()
{
}

// notice : task_info.stock_pinyin 's code will change to gbk
void WinnerWin::InsertIntoTbvTasklist(QTableView *tbv , T_TaskInformation &task_info)
{
    QStandardItemModel * model = static_cast<QStandardItemModel *>(tbv->model());
    model->insertRow(model->rowCount());
    int row_index = model->rowCount() - 1;
    auto align_way = Qt::AlignCenter;
    auto item = new QStandardItem( utility::FormatStr("%d", task_info.id).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_task_id, item);
    model->item(row_index, cst_tbview_tasks_rowindex_task_id)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_task_id, cst_small_width);

    // run state
	item = new QStandardItem(ToQString(static_cast<TaskCurrentState>(task_info.state)));
    model->setItem(row_index, cst_tbview_tasks_rowindex_state, item);
    model->item(row_index, cst_tbview_tasks_rowindex_state)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_state, cst_small_width);

	QString stock_str; 
    // task name
	if( task_info.type == TypeTask::INDEX_RISKMAN )
	{
		if( task_info.index_rel_task.rel_type == TindexTaskType::ALERT )
		{
			item = new QStandardItem(QString::fromLocal8Bit("ָ������Ԥ��"));
			stock_str = QString("%1/%2").arg(task_info.stock.c_str()).arg(QString::fromLocal8Bit(task_info.stock.c_str()));
		}else if( task_info.index_rel_task.rel_type == TindexTaskType::CLEAR )
		{
			item = new QStandardItem(QString::fromLocal8Bit("ָ���������"));
			stock_str = QString("%1/%2").arg(task_info.stock.c_str()).arg(QString::fromLocal8Bit(task_info.stock.c_str()));
		}else if( task_info.index_rel_task.rel_type == TindexTaskType::RELSTOCK )
		{
			//QString name_str = QString::fromLocal8Bit("ָ������") + task_info.index_rel_task.stock_code.c_str();
			item = new QStandardItem(QString::fromLocal8Bit("ָ����������") + task_info.index_rel_task.stock_code.c_str());
			stock_str = QString("%1").arg(task_info.index_rel_task.stock_code.c_str());
		}
	}else
	{
		item = new QStandardItem(ToQString(task_info.type)); 
		stock_str = QString("%1/%2").arg(task_info.stock.c_str()).arg(QString::fromLocal8Bit(task_info.stock_pinyin.c_str()));
	}
    model->setItem(row_index, cst_tbview_tasks_rowindex_task_name, item);
    model->item(row_index, cst_tbview_tasks_rowindex_task_name)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_task_name, cst_normal_width);

    //  stock name 
	item = new QStandardItem(stock_str);
    model->setItem(row_index, cst_tbview_tasks_rowindex_stock_name, item);
    model->item(row_index, cst_tbview_tasks_rowindex_stock_name)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_stock_name, cst_big_width);
    // current price
    item = new QStandardItem("");
    model->setItem(row_index, cst_tbview_tasks_rowindex_cur_price, item);
    model->item(row_index, cst_tbview_tasks_rowindex_cur_price)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_cur_price, cst_small_width);
    // trigger price 
    item = new QStandardItem( utility::FormatStr("%.2f", task_info.alert_price).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_trigger_price, item);
    model->item(row_index, cst_tbview_tasks_rowindex_trigger_price)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_trigger_price, cst_small_width);

    // quntity
    item = new QStandardItem( utility::FormatStr("%d", task_info.quantity).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_quantity, item);
    model->item(row_index, cst_tbview_tasks_rowindex_quantity)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_quantity, cst_small_width);
    // price level
    item = new QStandardItem( ToQString(static_cast<TypeQuoteLevel>(task_info.target_price_level)) );
    model->setItem(row_index, cst_tbview_tasks_rowindex_price_level, item);
    model->item(row_index, cst_tbview_tasks_rowindex_price_level)->setTextAlignment(align_way);
    // start time
    item = new QStandardItem( utility::FormatStr("%d", task_info.start_time).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_start_time, item);
    model->item(row_index, cst_tbview_tasks_rowindex_start_time)->setTextAlignment(align_way);
	tbv->setColumnWidth(cst_tbview_tasks_rowindex_start_time, cst_small_width);
    // end time
    item = new QStandardItem( utility::FormatStr("%d", task_info.end_time).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_end_time, item);
    model->item(row_index, cst_tbview_tasks_rowindex_end_time)->setTextAlignment(align_way);        
    tbv->setColumnWidth(cst_tbview_tasks_rowindex_end_time, cst_small_width);

    item = new QStandardItem( utility::FormatStr("%d", task_info.type).c_str() );
    model->setItem(row_index, cst_tbview_tasks_rowindex_task_type, item);
    model->item(row_index, cst_tbview_tasks_rowindex_task_type)->setTextAlignment(align_way);  

    QString str;
    if( task_info.type == TypeTask::EQUAL_SECTION )
        str = task_info.assistant_field.c_str();
    item = new QStandardItem(str);
    model->setItem(row_index, cst_tbview_tasks_rowindex_pretrigged, item);
    model->item(row_index, cst_tbview_tasks_rowindex_pretrigged)->setTextAlignment(align_way);  
}
 
void WinnerWin::Init()
{
	assert(app_->user_broker_info());

	QString title = QString("TradeWinner0.2 %1 %2").arg(QString::fromLocal8Bit(app_->user_broker_info()->remark.c_str())).arg(QString::fromLocal8Bit(app_->user_info().name.c_str()));
	setWindowTitle(title);

    flash_win_timer_ = new QTimer(this);
    flash_win_timer_->setSingleShot(false); 
    flash_win_timer_->setInterval(600);
    connect(flash_win_timer_, SIGNAL(timeout()), this, SLOT(DoFlashWin()));

    bool ret = connect(this->app_, SIGNAL(SigAppendLog(char*)), this, SLOT(SlotAppendLog(char*)));
	connect(ui.tabwid_holder, SIGNAL(currentChanged(int)), this, SLOT(SlotTabChanged(int)));

    ret = connect(ui.actionStopAllTask, SIGNAL(triggered(bool)), this->app_, SLOT(SlotStopAllTasks(bool)));
    
    // ndedt
    ret = connect(ui.actionOpenCalcWin, SIGNAL(triggered(bool)), this, SLOT(SlotOpenCalcWin(bool)));
    ui.tbview_tasks->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.tbview_tasks->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.tbview_tasks->setSelectionMode(QAbstractItemView::SingleSelection);
    
    //set menu context of tableview tasklist -------------------
    auto action_start = new QAction (this);
    action_start ->setText ( QStringLiteral( "����" ));
    connect(action_start, SIGNAL(triggered(bool)), this, SLOT(SlotTbvTasksActionStart(bool)));

    auto action_stop = new QAction (this);
    action_stop ->setText ( QStringLiteral( "ֹͣ" ));
    connect(action_stop, SIGNAL(triggered(bool)), this, SLOT(SlotTbvTasksActionStop(bool)));

    auto action_del = new QAction (this);
    action_del ->setText ( QStringLiteral( "ɾ��" ));
    bool ret_con_del = connect(action_del, SIGNAL(triggered(bool)), this, SLOT(SlotTbvTasksActionDel(bool)));

    auto action_detail = new QAction (this);
    action_detail ->setText ( QStringLiteral( "�鿴����" ));
    connect(action_detail, SIGNAL(triggered(bool)), this, SLOT(SlotTbvTasksActionDetail(bool)));

    tbv_tasks_popMenu_ = new QMenu(ui.tbview_tasks);
    tbv_tasks_popMenu_->addAction(action_start);
    tbv_tasks_popMenu_->addAction(action_stop);
    tbv_tasks_popMenu_->addAction(action_del);
    
    tbv_tasks_popMenu_->addAction(action_detail);
    
    bool ret_of_con = connect(ui.tbview_tasks, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(SlotTbvTasksContextMenu(QPoint))); 
    //-----------------------------------------

    std::for_each( std::begin(app_->task_infos()), std::end(app_->task_infos()), [&, this](WinnerApp::T_IdMapTaskInfo::reference entry)
    {
        InsertIntoTbvTasklist(ui.tbview_tasks, *entry.second);
    });

    // load log content -------------------
    for(int i = 0; i < 10; ++i)
    {
        std::ifstream input_file(utility::FormatStr("trade_winner/trade_winner.OrderData_%d.%d.log", Today(), i));
        if( !input_file ) 
           break;
        // get length of file:
        input_file.seekg (0, input_file.end);
        int length = input_file.tellg();
        input_file.seekg (0, input_file.beg);

        // allocate memory:
        auto buf = std::make_shared<Buffer>(length);  
        // read data as a block:
        input_file.read (buf->data(), length);
        input_file.close();
     
        ui.pte_log->appendPlainText(QString::fromLocal8Bit(buf->c_data()));
    }
    ret = connect(this->app_, SIGNAL(SigAddtionPrice(const QString&, const QString&)), this, SLOT(DoRecAdditionPrice(const QString&, const QString&)));
    ret = ret;
}

void WinnerWin::keyPressEvent(QKeyEvent *event)
{ 
    if( event->key() == Qt::Key_Escape )
    {
        if( ui.le_stock->hasFocus() )
        {
            m_list_hint_->close();
        }else if(ui.le_buytask_stock->hasFocus() )
        {
            m_bt_list_hint_->close();
		}else if( ui.le_eqsec_stock->hasFocus() )
		{
			m_eqsec_list_hint_->close();
        }else if( ui.le_adveq_stock->hasFocus() )
        {
            m_adveq_list_hint_->close();
        }

    }else
    {
        if( ui.le_stock->hasFocus() )
        {
            m_list_hint_->setFocus();
        }else if(ui.le_buytask_stock->hasFocus() )
        {
            m_bt_list_hint_->setFocus();
        }else if( ui.le_eqsec_stock->hasFocus() )
		{
			m_eqsec_list_hint_->setFocus();
		}else if( ui.le_adveq_stock->hasFocus() )
        {
            m_adveq_list_hint_->setFocus();
        }

    }
}


void WinnerWin::DoQueryCapital()
{ 
    ui.pte_capital->clear();
    T_CodeMapPosition pos_map = app_->QueryPosition();
    T_Capital captital = app_->QueryCapital();
    ui.pte_capital->appendPlainText(QString::fromLocal8Bit(utility::FormatStr("���ʲ�:\t%.2f\n", captital.total).c_str())); 
    ui.pte_capital->appendPlainText(QString::fromLocal8Bit("�ʽ�:\n"));
    ui.pte_capital->appendPlainText(QString::fromLocal8Bit(utility::FormatStr("����:\t%.2f\n", captital.available).c_str()));
    ui.pte_capital->appendPlainText(QString::fromLocal8Bit(utility::FormatStr("���:\t%.2f\n\n", captital.remain).c_str()));

    //ui.pte_capital->appendPlainText(QString::fromLocal8Bit("�ʽ�:\n"));

    ui.pte_capital->appendPlainText(QString::fromLocal8Bit("��Ʊ: \t\t����\t����\t����ӯ��\tӯ������\n"));
    std::for_each(std::begin(pos_map), std::end(pos_map), [this]( std::unordered_map<std::string, T_PositionData>::reference entry)
    {  
        auto str = QString::fromLocal8Bit(
            utility::FormatStr("%s\t%s\t%d\t%d\t%.2f\t%.2f\n", entry.first.c_str(), app_->db_moudle().GetStockName(entry.first).c_str()
                , entry.second.total, entry.second.avaliable, entry.second.profit, entry.second.profit_percent).c_str()  );
        qDebug() << str << " -------\n";
        ui.pte_capital->appendPlainText(str);
         
    });
}

void WinnerWin::SlotAppendLog(char *p_chars)
{
    ui.pte_log->appendPlainText(QString::fromLocal8Bit(p_chars));
    delete p_chars; p_chars = nullptr;
}

void WinnerWin::SlotTabChanged(int /*index*/)
{
	assert(m_list_hint_ && m_bt_list_hint_);
	m_list_hint_->hide();
    m_bt_list_hint_->hide();
    m_eqsec_list_hint_->hide();
    m_adveq_list_hint_->hide();
    
}

void WinnerWin::SlotTbvTasksContextMenu(QPoint p)
{
    QModelIndex pmindex = ui.tbview_tasks->indexAt(p);
    if( pmindex.isValid() )
    {
        tbv_tasks_popMenu_->exec(QCursor::pos());
    }
    ui.tbview_tasks->currentIndex();
}

void WinnerWin::SlotTbvTasksActionStart(bool)
{ 
    auto task_id = TbvTasksCurRowTaskId();
     
    auto strategy_task = app_->FindStrategyTask(task_id); 
    assert(strategy_task);

    if( strategy_task->cur_state() == static_cast<TaskCurrentState>(TaskStateElem::STOP) )
    { 
		strategy_task->set_a_state(TaskStateElem::WAITTING); //strategy_task->cur_state(TaskCurrentState::WAITTING);
        this->app_->Emit(strategy_task.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
        auto cur_time = QTime::currentTime();
        // in task time
        if( cur_time >= strategy_task->tp_start() && cur_time <= strategy_task->tp_end() )
        {
            app_->ticker_strand().PostTask([strategy_task, this]()
            {
            this->app_->local_logger().LogLocal(utility::FormatStr("SlotTbvTasksActionStart %d", strategy_task->task_id()));
            this->app_->stock_ticker().Register(strategy_task);
            });
			 
            if( IsNowTradeTime() )
			{ 
                strategy_task->set_a_state(TaskStateElem::STARTING);//strategy_task->cur_state(TaskCurrentState::STARTING);
			}else
			{
                strategy_task->set_a_state(TaskStateElem::REST);
			}
            this->app_->Emit(strategy_task.get(), static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
        } 
    }

    if( !strategy_task->is_to_run() )
    {
      strategy_task->SetOriginalState( static_cast<TaskCurrentState>(TaskStateElem::WAITTING) );
      app_->db_moudle().UpdateTaskInfo(strategy_task->task_info());
    }
}

void WinnerWin::SlotTbvTasksActionStop(bool)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
    QModelIndex mod_index = ui.tbview_tasks->currentIndex();
    auto task_id = model->item(mod_index.row(), cst_tbview_tasks_rowindex_task_id)->text().toInt();
    auto strategy_task = app_->FindStrategyTask(task_id);
    assert(strategy_task);

    if( strategy_task->is_to_run() )
    {
        // change record state in db
		strategy_task->SetOriginalState(static_cast<TaskCurrentState>(TaskStateElem::STOP));
        app_->db_moudle().UpdateTaskInfo(strategy_task->task_info());
    }
    this->app_->local_logger().LogLocal(utility::FormatStr("SlotTbvTasksActionStop step1 %d", task_id));

    // unregisterd task
    auto p_strategy_task = strategy_task.get();
    app_->ticker_strand().PostTask([p_strategy_task, task_id, this]()
    {
        this->app_->local_logger().LogLocal(utility::FormatStr("SlotTbvTasksActionStop step2 %d", task_id));
        p_strategy_task->cur_state(static_cast<TaskCurrentState>(TaskStateElem::STOP));
        this->app_->Emit(p_strategy_task, static_cast<int>(TaskStatChangeType::CUR_STATE_CHANGE));
        this->app_->stock_ticker().UnRegister(task_id);
        
    });

    model->submit();
    update();

}

void WinnerWin::SlotTbvTasksActionDel(bool)
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
    QModelIndex mod_index = ui.tbview_tasks->currentIndex();
    auto task_id = model->item(mod_index.row(), cst_tbview_tasks_rowindex_task_id)->text().toInt();
    auto task_type = model->item(mod_index.row(), cst_tbview_tasks_rowindex_task_type)->text().toInt();
    
    app_->DelTaskById(task_id, (TypeTask)task_type);

    model->removeRow(mod_index.row());
    model->submit();
    update();
    return;
}

void WinnerWin::SlotTbvTasksActionDetail(bool)
{    
    auto task_id = TbvTasksCurRowTaskId();
    DoShowTaskDetail(task_id);

#if 0
    auto p_tskinfo = app_->FindTaskInfo(task_id);
    if( !p_tskinfo )
        return;

	is_open_hint_ = false;
     
    switch(p_tskinfo->type)
    {
    case TypeTask::BREAK_SELL: 
    case TypeTask::INFLECTION_SELL:
    case TypeTask::FOLLOW_SELL:
    case TypeTask::BATCHES_SELL:
        cur_price_ = 0.0;
        FillSellTaskWin(p_tskinfo->type, *p_tskinfo);
		ui.tabwid_holder->setCurrentIndex(cst_tab_index_sell_task);
        break;
    case TypeTask::BREAKUP_BUY:
    case TypeTask::INFLECTION_BUY:
    case TypeTask::BATCHES_BUY:
        buytask_cur_price_ = 0.0;
        FillBuyTaskWin(p_tskinfo->type, *p_tskinfo);
		ui.tabwid_holder->setCurrentIndex(cst_tab_index_buy_task);
        break;
    case TypeTask::EQUAL_SECTION:
        {
            ui.le_eqsec_stock->setText( QString("%1/%2").arg(p_tskinfo->stock.c_str()).arg(QString::fromLocal8Bit(p_tskinfo->stock_pinyin.c_str())) );
            
            ui.dbspbox_eqsec_start_price->setValue(p_tskinfo->alert_price);
            ui.dbspbox_eqsec_raise_percent->setValue(p_tskinfo->secton_task.raise_percent);
            ui.dbspbox_eqsec_fall_percent->setValue(p_tskinfo->secton_task.fall_percent);
            ui.spinBox_eqsec_quantity->setValue(p_tskinfo->quantity);
             
            if( p_tskinfo->secton_task.max_position == EQSEC_MAX_POSITION )
                ui.cb_max_qty->setChecked(false);
            else
                ui.cb_max_qty->setChecked(true);
            if( p_tskinfo->secton_task.min_position == EQSEC_MIN_POSITION )
                ui.cb_min_qty->setChecked(false);
            else
                ui.cb_min_qty->setChecked(true);
			if( p_tskinfo->rebounce > 0.0 )
			{
				ui.wid_eqsec_rebounce_grp->setEnabled(true);
				ui.cb_eqsec_rebounce->setChecked(true);
				ui.spinBox_eqsec_rebounce->setValue(p_tskinfo->rebounce);
				if( p_tskinfo->back_alert_trigger )
					ui.cb_eqsec_back_trigger->setChecked(true);
				else
					ui.cb_eqsec_back_trigger->setChecked(false);
			}else
			{
				ui.wid_eqsec_rebounce_grp->setDisabled(true);
				ui.cb_eqsec_rebounce->setChecked(false);
				ui.spinBox_eqsec_rebounce->setValue(0.0);
				ui.cb_eqsec_back_trigger->setChecked(false);
			}
			
            ui.spinBox_max_qty->setValue(p_tskinfo->secton_task.max_position);
            ui.spinBox_min_qty->setValue(p_tskinfo->secton_task.min_position);

            if( Equal(p_tskinfo->secton_task.max_trig_price, EQSEC_MAX_STOP_PRICE) )
                ui.cb_max_stop_trigger->setChecked(false);
            else
                ui.cb_max_stop_trigger->setChecked(true);
            if( Equal(p_tskinfo->secton_task.min_trig_price, EQSEC_MIN_CLEAR_PRICE) )
                ui.cb_min_clear_trigger->setChecked(false);
            else
                ui.cb_min_clear_trigger->setChecked(true);
             
            ui.dbspbox_eqsec_max_price->setValue(p_tskinfo->secton_task.max_trig_price);
            ui.dbspbox_eqsec_min_price->setValue(p_tskinfo->secton_task.min_trig_price);

            ui.combox_eqsec_price_level->setCurrentText(ToQString(static_cast<TypeQuoteLevel>(p_tskinfo->target_price_level)));
            
            ui.timeEdit_eqsec_begin->setTime(Int2Qtime(p_tskinfo->start_time));
            ui.timeEdit_eqsec_end->setTime(Int2Qtime(p_tskinfo->end_time));
            ui.tabwid_holder->setCurrentIndex(cst_tab_index_eqsec_task);
        }
        break;
	case TypeTask::INDEX_RISKMAN:
        {
			ui.combox_stkindex->setCurrentText(IndexCode2IndexName(p_tskinfo->stock.c_str()));
             
			ui.dbspbox_index_val->setValue(p_tskinfo->alert_price);
			ui.radiobtn_cross_down->setChecked( p_tskinfo->index_rel_task.is_down_trigger );
			switch(p_tskinfo->index_rel_task.rel_type)
			{
			case TindexTaskType::ALERT: ui.radiobtn_alert->setChecked(true); DoTrdIndexAlertBtnBtnChecked(true);break;
			case TindexTaskType::RELSTOCK: ui.radiobtn_reltrade->setChecked(true); DoTrdIndexRelBtnBtnChecked(true); break;
			case TindexTaskType::CLEAR: ui.radiobtn_clearall->setChecked(true); DoTrdIndexClearBtnChecked(true);break;
			default: assert(false);
			}
            ui.indtrd_timeEdit_begin->setTime(Int2Qtime(p_tskinfo->start_time));
            ui.indtrd_timeEdit_end->setTime(Int2Qtime(p_tskinfo->end_time));
            ui.tabwid_holder->setCurrentIndex(cst_tab_index_stkindex_task);
		}
		break;
    }
     

EXIT_PROC:

	is_open_hint_ = true;
#endif 
}

void WinnerWin::RemoveByTaskId(int task_id)
{ 
    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
    int i = 0;
    for( ; i < model->rowCount(); ++i )
    {
        if( task_id == model->item(i, cst_tbview_tasks_rowindex_task_id)->text().toInt() )
           break;
    }
    if( i < model->rowCount() )
    {
        model->removeRow(i);
        model->submit();
        update();
    }
    
}

void WinnerWin::DoStatusBar(const std::string& str)
{
    status_label_->setText(QString::fromLocal8Bit(str.c_str()));
}

void WinnerWin::DoStatusBar(std::string* str, bool is_delete)
{
    assert(str);
    status_label_->setText(QString::fromLocal8Bit(str->c_str()));
    if( is_delete )
        delete str;
}

void WinnerWin::DoFlashWin()
{
    FlashWindow( HWND(this->winId()), 1);
}

void WinnerWin::DoRecAdditionPrice(const QString& code, const QString& prices_str)
{
    std::vector<std::string> quotas = TSystem::utility::split(prices_str.toLocal8Bit().data(), ";");
    if( buy_task_win_addtion_code_ == code )
    {
        ui.lab_bw_out_buy1->setText(quotas[0].c_str());
        ui.lab_bw_out_sell1->setText(quotas[1].c_str());
        ui.lab_bw_out_buy2->setText(quotas[2].c_str());
        ui.lab_bw_out_sell2->setText(quotas[3].c_str());
        ui.lab_bw_out_buy3->setText(quotas[4].c_str());
        ui.lab_bw_out_sell3->setText(quotas[5].c_str());
        ui.lab_bw_out_buy4->setText(quotas[6].c_str());
        ui.lab_bw_out_sell4->setText(quotas[7].c_str());
        ui.lab_bw_out_buy5->setText(quotas[8].c_str());
        ui.lab_bw_out_sell5->setText(quotas[9].c_str());
    }
    if( sell_task_win_addtion_code_ == code )
    {
        ui.lab_out_buy1->setText(quotas[0].c_str());
        ui.lab_out_sell1->setText(quotas[1].c_str());
        ui.lab_out_buy2->setText(quotas[2].c_str());
        ui.lab_out_sell2->setText(quotas[3].c_str());
        ui.lab_out_buy3->setText(quotas[4].c_str());
        ui.lab_out_sell3->setText(quotas[5].c_str());
        ui.lab_out_buy4->setText(quotas[6].c_str());
        ui.lab_out_sell4->setText(quotas[7].c_str());
        ui.lab_out_buy5->setText(quotas[8].c_str());
        ui.lab_out_sell5->setText(quotas[9].c_str());
    }
    
}

void WinnerWin::DoLeStockEditingFinished()
{
    if( !ui.le_stock->isModified() )
        return;

    auto val = ui.le_stock->isModified();
    ui.le_stock->setModified(false);
    //ui.te_stock_match->hide();

#if 0
    auto ckval =  ui.le_stock->text();
    auto stock_code = ui.le_stock->text().trimmed();
    if( stock_code.length() != 6 )
        return;

    byte category = 9;
    int market = stock_code.at(0) == "6" ? 1 : 0;

    auto zqdm = stock_code.toLocal8Bit().data();

    //char  result[1024*1024] = {0};
    auto result = std::make_shared<Buffer>(1024*1024);
    char error[2*1024] = {0};
    short count = 1;
   // app_->stock_ticker().GetSecurityBars(category, market, stock_code.toLocal8Bit().data(), 0, count, result->data(), error);
#endif
}

void WinnerWin::DoLeStockChanged(const QString &str)
{
    if( str.length() > 0 )
    {
        /*auto temp = ui.le_stock->text();
        ui.te_stock_match->append(temp);

        std::vector<T_StockCodeName> stockCodeNames;
        app_->db_moudle().GetStockCode(str.toLocal8Bit().data(), stockCodeNames);*/
    }
}

void WinnerWin::DoSpboxQuantityEditingFinished()
{
    
    //ui.spinBox_quantity->mo
}

void WinnerWin::closeEvent(QCloseEvent * event)
{
    auto ret_button = QMessageBox::question(nullptr, QString::fromLocal8Bit("��ʾ"), QString::fromLocal8Bit("ȷ���˳�ϵͳ?"),
        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if( ret_button == QMessageBox::Cancel )
        event->ignore();
    else
        app_->Stop();
}

void WinnerWin::changeEvent(QEvent * event)
{ // if( event->type() == QEvent::WindowStateChange )
    if( flash_win_timer_ && flash_win_timer_->isActive() )  
        flash_win_timer_->stop();
}

void WinnerWin::DoTaskStatChangeSignal(StrategyTask* p_task, int val)
{
    assert(p_task);
	static auto find_target_index = [this](int task_id)->int
	{
		for( int i = 0; i < ui.tbview_tasks->model()->rowCount(); ++i )
        {
            if( task_id == static_cast<QStandardItemModel *>(ui.tbview_tasks->model())->item(i, cst_tbview_tasks_rowindex_task_id)->text().toUInt() ) 
            {
                return i;
            }
        }
		return -1;
	};

	QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
    switch(static_cast<TaskStatChangeType>(val))
    {
    case TaskStatChangeType::CUR_PRICE_CHANGE:
        {
            /*for( int i = 0; i < ui.tbview_tasks->model()->rowCount(); ++i )
            {
                if( p_task->task_id() == static_cast<QStandardItemModel *>(ui.tbview_tasks->model())->item(i, cst_tbview_tasks_rowindex_task_id)->text().toUInt() ) 
                {
                    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
                    model->item(i, cst_tbview_tasks_rowindex_cur_price)->setText(utility::FormatStr("%.2f", p_task->cur_price()).c_str());
                    return;
                }
            }*/
			auto i = find_target_index(p_task->task_id());
			if( i >= 0 )
			{
				model->item(i, cst_tbview_tasks_rowindex_cur_price)->setText(utility::FormatStr("%.2f", p_task->cur_price()).c_str());
                return;
			}
            break;
        }
    case TaskStatChangeType::CUR_STATE_CHANGE:
        {
            /*for( int i = 0; i < ui.tbview_tasks->model()->rowCount(); ++i )
            {
                if( p_task->task_id() == static_cast<QStandardItemModel *>(ui.tbview_tasks->model())->item(i, cst_tbview_tasks_rowindex_task_id)->text().toUInt() ) 
                {
                    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
                    model->item(i, cst_tbview_tasks_rowindex_state)->setText(ToQString(p_task->cur_state()));
                    return;
                }
            }*/
			auto i = find_target_index(p_task->task_id());
			if( i >= 0 )
			{ 
				model->item(i, cst_tbview_tasks_rowindex_state)->setText(ToQString(p_task->cur_state()));
                return;
			}
            break; 
        }
	case TaskStatChangeType::PRE_TRIGG_PRICE_CHANGE:
		{
			auto i = find_target_index(p_task->task_id());
			if( i >= 0 )
			{ 
                QString str = QString("%1").arg(p_task->pre_trigged_price());
				model->item(i, cst_tbview_tasks_rowindex_pretrigged)->setText(str);
                return;
			}
            break; 
		}
    }

}

void WinnerWin::DoQueryPosition()
{
    QString text = ui.le_stock->text();
    QString::SectionFlag flag = QString::SectionSkipEmpty;
    QString tgt_tag = text.section('/', 0, 0, flag);

    if( IsStrNum(tgt_tag.toLocal8Bit().data()) && tgt_tag.length() == 6 )
    {
        auto pos = app_->QueryPosition(tgt_tag.toLocal8Bit().data());
        if( pos )
        {
            ui.label_total_num->setText(QString("%1").arg(pos->avaliable));
            ui.spinBox_quantity->setValue(pos->avaliable);
        }
    }
}

int WinnerWin::TbvTasksCurRowTaskId()
{
    QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
    return model->item(ui.tbview_tasks->currentIndex().row(), cst_tbview_tasks_rowindex_task_id)->text().toInt();

}

void WinnerWin::FlushFromStationListWidget(QString str)
{
	if( !is_open_hint_ )
		return;
     
    HintList *p_list = nullptr;
    QLineEdit *p_edit = nullptr;
    QDoubleSpinBox *p_dbspb_price = nullptr;
    QDoubleSpinBox *p_dbspb_percent = nullptr;
    AssignHintListAndLineEdit(p_list, p_edit, p_dbspb_price, p_dbspb_percent);

    p_list->clear();
	 
    //qDebug() << "FlushFromStationListWidget " << str << "\n";

    std::vector<T_StockCodeName> stockCodeNames;
    app_->db_moudle().GetStockCode(str.toLocal8Bit().data(), stockCodeNames);
     
    std::for_each( std::begin(stockCodeNames), std::end(stockCodeNames), [p_list, this](T_StockCodeName& entry)
    { 
        /*auto str = QTextCodec::codecForName( "utf-8" )->toUnicode(entry.name.c_str()); 
        QString tmp_str = QString("%1/%2").arg(entry.code.c_str()).arg(QString::fromLocal8Bit(entry.name.c_str()));
*/
        p_list->addItem( QString("%1/%2").arg(entry.code.c_str()).arg(QString::fromLocal8Bit(entry.name.c_str())) );
    });
    if( p_list->count() > 0 )
    {
        int x,y, w,h;
        p_edit->geometry().getRect(&x, &y, &w, &h);
        if( ui.tabwid_holder->currentIndex() != cst_tab_index_stkindex_task )
        {   // grp_relstock_trade
            p_list->setGeometry( x + 4, y + 90, w, h + 40);
        }else
        {
            int x_g, y_g, w_g, h_g;
            ui.grp_relstock_trade->geometry().getRect(&x_g, &y_g, &w_g, &h_g);
            p_list->setGeometry( x_g + x + 4, y_g + y + 90, w, h + 40);
        }
        p_list->setCurrentRow(0);
        p_list->ShowUI();
    }
} 

void WinnerWin::OnClickedListWidget(QModelIndex index)
{
    HintList *p_list = nullptr;
    QLineEdit *p_edit = nullptr;
    QDoubleSpinBox *p_dbspb_price = nullptr;
    QDoubleSpinBox *p_dbspb_percent = nullptr;
    AssignHintListAndLineEdit(p_list, p_edit, p_dbspb_price, p_dbspb_percent);

    QString text = p_list->item(index.row())->text();
    ChangeFromStationText(text);
     
}

void WinnerWin::ChangeFromStationText(QString text)
{
    HintList *p_list = nullptr;
    QLineEdit *p_edit = nullptr;
    QDoubleSpinBox *p_dbspb_price = nullptr;
    QDoubleSpinBox *p_dbspb_percent = nullptr;
    AssignHintListAndLineEdit(p_list, p_edit, p_dbspb_price, p_dbspb_percent);
     
    p_edit->setText(text);
    p_list->hide();

    //qDebug() << "ChangeFromStationText " << tgt_tag << "\n";
	 
    QString::SectionFlag flag = QString::SectionSkipEmpty;
    QString tgt_tag = text.section('/', 0, 0, flag);
    std::string code = tgt_tag.toLocal8Bit().data();
    if( !IsStrNum(code) || code.size() != 6 )
        return;

	T_StockPriceInfo *p_info = app_->GetStockPriceInfo(code, false);
	if( p_info )
    {
		if( p_dbspb_price )
			p_dbspb_price->setValue(p_info->cur_price);
        if( p_dbspb_percent )
            p_dbspb_percent->setValue(0.0);

        if( ui.tabwid_holder->currentIndex() == cst_tab_index_sell_task )
            cur_price_ = p_info->cur_price;
        else if( ui.tabwid_holder->currentIndex() == cst_tab_index_buy_task )
            buytask_cur_price_ = p_info->cur_price;
		/*else if( ui.tabwid_holder->currentIndex() == cst_tab_advsec_task )
            adveq_cur_price_ = p_info->cur_price; */
    }else
    {
        if( ui.tabwid_holder->currentIndex() == cst_tab_index_sell_task )
            cur_price_ = 0.0;
        else if( ui.tabwid_holder->currentIndex() == cst_tab_index_buy_task )
            buytask_cur_price_ = 0.0;
		else if( ui.tabwid_holder->currentIndex() == cst_tab_index_eqsec_task )
			eqsec_task_cur_price_ = 0.0;
    }
    if( ui.tabwid_holder->currentIndex() == cst_buytask_tab_index && buy_task_win_addtion_code_ != code.c_str() )
    {
        DoBuyWinAddtionCodeChange(code);
    }else if( ui.tabwid_holder->currentIndex() == cst_selltask_tab_index && sell_task_win_addtion_code_ != code.c_str() )
    {
        DoSellWinAddtionCodeChange(code);
    }
    
}

void WinnerWin::DoSellWinAddtionCodeChange(const std::string& code)
{
    if( !sell_task_win_addtion_code_.isEmpty() 
        && buy_task_win_addtion_code_ != sell_task_win_addtion_code_ )
        this->app_->UnRegAddtionPrice(sell_task_win_addtion_code_.toLocal8Bit().data());
    sell_task_win_addtion_code_ = code.c_str();
    this->app_->RegisterAddtionPrice(code);
}

void WinnerWin::DoBuyWinAddtionCodeChange(const std::string& code)
{
    if( !buy_task_win_addtion_code_.isEmpty() 
        && buy_task_win_addtion_code_ != sell_task_win_addtion_code_ )
        this->app_->UnRegAddtionPrice(buy_task_win_addtion_code_.toLocal8Bit().data());
    buy_task_win_addtion_code_ = code.c_str();
    this->app_->RegisterAddtionPrice(code);
}

void WinnerWin::AssignHintListAndLineEdit(HintList *& p_list, QLineEdit *&p_edit, QDoubleSpinBox *&p_dbspb_alert_price, QDoubleSpinBox *&p_dbspb_percent)
{
    if( ui.tabwid_holder->currentIndex() == cst_tab_index_sell_task )
    {
        p_list = m_list_hint_;
        p_edit = ui.le_stock;
        p_dbspb_alert_price = ui.dbspbox_alert_price;
        p_dbspb_percent = ui.dbspbox_alert_percent;
    }else if( ui.tabwid_holder->currentIndex() == cst_tab_index_buy_task )
    {   
        p_list = m_bt_list_hint_;
        p_edit = ui.le_buytask_stock;
        p_dbspb_alert_price = ui.dbspbox_buytask_alert_price;
        p_dbspb_percent = ui.dbspbox_buytask_alert_percent;
    }else if( ui.tabwid_holder->currentIndex() == cst_tab_index_eqsec_task )
    {   
        p_list = m_eqsec_list_hint_;
        p_edit = ui.le_eqsec_stock;
        p_dbspb_alert_price = ui.dbspbox_eqsec_start_price;
        p_dbspb_percent = nullptr;
    }else if( ui.tabwid_holder->currentIndex() == cst_tab_index_adveq_task )
    {   
        p_list = m_adveq_list_hint_;
        p_edit = ui.le_adveq_stock;
        //p_dbspb_alert_price = ui.dbspbox_eqsec_start_price;
        p_dbspb_percent = nullptr;
    }else if( ui.tabwid_holder->currentIndex() == cst_tab_index_stkindex_task )
    {   
        p_list = m_indtrd_list_hint_;
        p_edit = ui.le_indtrd_stock;
        
    }else 
        assert(false);
}

void WinnerWin::SlotOpenCalcWin(bool)
{
    if( !calc_win_ )
        calc_win_ = std::make_shared<CalcWin>();

    ::SetWindowPos(HWND(calc_win_->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    //::SetWindowPos(HWND(calc_win_->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW); 
    calc_win_->show();
    calc_win_->activateWindow();
}

void WinnerWin::TriggerFlashWinTimer(bool enable)
{
    if( enable )
        flash_win_timer_->start();
    else
        flash_win_timer_->stop();
}

void WinnerWin::DoTabTasksDbClick(const QModelIndex index)
{
     QStandardItemModel *model = static_cast<QStandardItemModel *>(ui.tbview_tasks->model());
     int task_id = model->item(index.row(), cst_tbview_tasks_rowindex_task_id)->text().toInt();
     DoShowTaskDetail(task_id);
}

void WinnerWin::DoShowTaskDetail(int task_id)
{
    auto p_tskinfo = app_->FindTaskInfo(task_id);
    if( !p_tskinfo )
        return;

	is_open_hint_ = false;
     
    switch(p_tskinfo->type)
    {
    case TypeTask::BREAK_SELL: 
    case TypeTask::INFLECTION_SELL:
    case TypeTask::FOLLOW_SELL:
    case TypeTask::BATCHES_SELL:
        cur_price_ = 0.0;
        FillSellTaskWin(p_tskinfo->type, *p_tskinfo);
		ui.tabwid_holder->setCurrentIndex(cst_tab_index_sell_task);
        DoSellWinAddtionCodeChange(p_tskinfo->stock);
        break;
    case TypeTask::BREAKUP_BUY:
    case TypeTask::INFLECTION_BUY:
    case TypeTask::BATCHES_BUY:
        buytask_cur_price_ = 0.0;
        FillBuyTaskWin(p_tskinfo->type, *p_tskinfo);
		ui.tabwid_holder->setCurrentIndex(cst_tab_index_buy_task);
        DoBuyWinAddtionCodeChange(p_tskinfo->stock);
        break;
    case TypeTask::EQUAL_SECTION:
        {
            ui.le_eqsec_stock->setText( QString("%1/%2").arg(p_tskinfo->stock.c_str()).arg(QString::fromLocal8Bit(p_tskinfo->stock_pinyin.c_str())) );
            
            ui.dbspbox_eqsec_start_price->setValue(p_tskinfo->alert_price);
            ui.dbspbox_eqsec_raise_percent->setValue(p_tskinfo->secton_task.raise_percent);
            ui.dbspbox_eqsec_fall_percent->setValue(p_tskinfo->secton_task.fall_percent);
            ui.spinBox_eqsec_quantity->setValue(p_tskinfo->quantity);
             
            if( p_tskinfo->secton_task.max_position == EQSEC_MAX_POSITION )
                ui.cb_max_qty->setChecked(false);
            else
                ui.cb_max_qty->setChecked(true);
            if( p_tskinfo->secton_task.min_position == EQSEC_MIN_POSITION )
                ui.cb_min_qty->setChecked(false);
            else
                ui.cb_min_qty->setChecked(true);
			if( p_tskinfo->rebounce > 0.0 )
			{
				ui.wid_eqsec_rebounce_grp->setEnabled(true);
				ui.cb_eqsec_rebounce->setChecked(true);
				ui.spinBox_eqsec_rebounce->setValue(p_tskinfo->rebounce);
				if( p_tskinfo->back_alert_trigger )
					ui.cb_eqsec_back_trigger->setChecked(true);
				else
					ui.cb_eqsec_back_trigger->setChecked(false);
			}else
			{
				ui.wid_eqsec_rebounce_grp->setDisabled(true);
				ui.cb_eqsec_rebounce->setChecked(false);
				ui.spinBox_eqsec_rebounce->setValue(0.0);
				ui.cb_eqsec_back_trigger->setChecked(false);
			}
			
            ui.spinBox_max_qty->setValue(p_tskinfo->secton_task.max_position);
            ui.spinBox_min_qty->setValue(p_tskinfo->secton_task.min_position);

            if( Equal(p_tskinfo->secton_task.max_trig_price, EQSEC_MAX_STOP_PRICE) )
                ui.cb_max_stop_trigger->setChecked(false);
            else
                ui.cb_max_stop_trigger->setChecked(true);
            if( Equal(p_tskinfo->secton_task.min_trig_price, EQSEC_MIN_CLEAR_PRICE) )
                ui.cb_min_clear_trigger->setChecked(false);
            else
                ui.cb_min_clear_trigger->setChecked(true);
             
            ui.dbspbox_eqsec_max_price->setValue(p_tskinfo->secton_task.max_trig_price);
            ui.dbspbox_eqsec_min_price->setValue(p_tskinfo->secton_task.min_trig_price);

            ui.combox_eqsec_price_level->setCurrentText(ToQString(static_cast<TypeQuoteLevel>(p_tskinfo->target_price_level)));
            
            ui.timeEdit_eqsec_begin->setTime(Int2Qtime(p_tskinfo->start_time));
            ui.timeEdit_eqsec_end->setTime(Int2Qtime(p_tskinfo->end_time));
            ui.tabwid_holder->setCurrentIndex(cst_tab_index_eqsec_task);
        }
        break;
    case TypeTask::ADVANCE_SECTION:
        {
        ui.le_adveq_stock->setText(QString("%1/%2").arg(p_tskinfo->stock.c_str()).arg(QString::fromLocal8Bit(p_tskinfo->stock_pinyin.c_str())) );
       // p_tskinfo->advance_section_task.portion_sections.size();
        
        auto str_portion_vector = utility::split(p_tskinfo->advance_section_task.portion_sections, ";");
        if( str_portion_vector.size() && str_portion_vector.at(str_portion_vector.size()-1) == "" )
            str_portion_vector.pop_back();

        auto max_val = std::stod( str_portion_vector.at(str_portion_vector.size()-1) );
        ui.dbspb_adveq_max_price->setValue(max_val);

        auto min_val = std::stod( str_portion_vector.at(0) );
        ui.dbspb_adveq_min_price->setValue(min_val);

        ui.spb_adveq_section_count->setValue(str_portion_vector.size()-1);
        ui.spb_adveq_rebounce->setValue(p_tskinfo->rebounce);

        ui.spinBox_adveq_qty->setValue(p_tskinfo->quantity);
        ui.combox_adveq_price_level->setCurrentText(ToQString(static_cast<TypeQuoteLevel>(p_tskinfo->target_price_level)));
        ui.timeEdit_adveq_begin->setTime(Int2Qtime(p_tskinfo->start_time));
        ui.timeEdit_adveq_end->setTime(Int2Qtime(p_tskinfo->end_time));
        ui.tabwid_holder->setCurrentIndex(cst_tab_index_adveq_task);
        break;
        }
	case TypeTask::INDEX_RISKMAN:
        {
			ui.combox_stkindex->setCurrentText(IndexCode2IndexName(p_tskinfo->stock.c_str()));
             
			ui.dbspbox_index_val->setValue(p_tskinfo->alert_price);
			ui.radiobtn_cross_down->setChecked( p_tskinfo->index_rel_task.is_down_trigger );
			switch(p_tskinfo->index_rel_task.rel_type)
			{
			case TindexTaskType::ALERT: ui.radiobtn_alert->setChecked(true); DoTrdIndexAlertBtnBtnChecked(true);break;
			case TindexTaskType::RELSTOCK: ui.radiobtn_reltrade->setChecked(true); DoTrdIndexRelBtnBtnChecked(true); break;
			case TindexTaskType::CLEAR: ui.radiobtn_clearall->setChecked(true); DoTrdIndexClearBtnChecked(true);break;
			default: assert(false);
			}
            ui.indtrd_timeEdit_begin->setTime(Int2Qtime(p_tskinfo->start_time));
            ui.indtrd_timeEdit_end->setTime(Int2Qtime(p_tskinfo->end_time));
            ui.tabwid_holder->setCurrentIndex(cst_tab_index_stkindex_task);
		}
		break;
    }
     

EXIT_PROC:

	is_open_hint_ = true;
}