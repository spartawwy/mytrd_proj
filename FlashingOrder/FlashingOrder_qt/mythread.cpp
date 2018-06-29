#include "mythread.h"

#include <qt_windows.h>  

#include <QDebug>
#include <QString>

#include "flashingorder.h"

#define KEYDOWN(vk_code) ((GetKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define KEYUP(vk_code) ((GetKeyState(vk_code) & 0x8000) ? 0 : 1) 

MyThread::MyThread(FlashingOrder *app)
	: app_(app)
{ 
}

void MyThread::run()
{
	bool is_prepare_buy = false;
	bool is_prepare_sell = false;

	while ( !app_->exit_flag() )
	{
		QMutex & mt = app_->key_sig_mutex();

		mt.lock();
		app_->key_sig_wait_cond().wait(&mt);
		msleep(2); // it's neccery other wise can't get correctry key status
		
		// ---debug -----
		auto lctr_val = KEYUP(VK_LCONTROL);
		auto rctr_val = KEYUP(VK_RCONTROL);
		qDebug() << "my thread line 67 | lctr_val: " << lctr_val << " rctr_val: " << rctr_val << "\n";
        WriteLog("Enter MyThread");
        app_->EmitLog2Ui(QString("Enter MyThread %1").arg(stock_name_) );
		//----end ---------
		if( (KEYDOWN(VK_LCONTROL) || KEYDOWN(VK_RCONTROL)) && !(KEYDOWN(VK_LCONTROL) && KEYDOWN(VK_RCONTROL)) )
		{ 
			if( KEYDOWN(VK_OEM_PLUS) && KEYUP(VK_OEM_MINUS) )
				is_prepare_buy = true;
			else if( KEYDOWN(VK_OEM_MINUS) && KEYUP(VK_OEM_PLUS) )
				is_prepare_sell = true;
			else if( KEYDOWN(VK_OEM_MINUS) && KEYDOWN(VK_OEM_PLUS) )
			{
				is_prepare_buy = false;
				is_prepare_sell = false;
			}

		}
		if( KEYUP(VK_OEM_PLUS) )
		{
			if (is_prepare_buy && KEYUP(VK_OEM_MINUS))
			{
				is_prepare_buy = false;
                WriteLog("Triger buy %s", stock_name_.toLocal8Bit().data());
                qDebug() << "Triger buy\n";
				app_->HandleOrder(true, stock_name_.toLocal8Bit().data());
				
			} 
		}
		if( KEYUP(VK_OEM_MINUS) )
		{
			if (is_prepare_sell && KEYUP(VK_OEM_PLUS))
			{
				is_prepare_sell = false; 
                WriteLog("Triger sell %s", stock_name_.toLocal8Bit().data());
				qDebug() << "Triger sell\n";
				app_->HandleOrder(false, stock_name_.toLocal8Bit().data());
			}

		}
		if(  KEYUP(VK_LCONTROL) && KEYUP(VK_RCONTROL) )
		{ 
			qDebug() << "set all false\n";
			is_prepare_buy = false;
			is_prepare_sell = false;
		}	
		mt.unlock();
	}// while
}
 