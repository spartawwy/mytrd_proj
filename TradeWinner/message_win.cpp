#include "message_win.h"

#include <Windows.h>

#include <QTimer>

MessageWin::MessageWin()
    : QWidget()
{
    ui.setupUi(this);
     
    myTimer_ = new QTimer();
    myTimer_->start(5000);

    bool ret = connect(myTimer_,SIGNAL(timeout()),this,SLOT(SlotTimeout()));
 
}

void MessageWin::SetTitle(const QString &str)
{
    setWindowIconText(str);
}

void MessageWin::SetContent(const QString &str)
{
    ui.label_content->setText(str);
}

void MessageWin::ShowUI(const QString &title_str, const QString &str)
{
    setWindowIconText(title_str);
    ui.label_content->setText(str);
    ::SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    //::SetWindowPos(HWND(pMainForm->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW); 
    this->show();

    myTimer_->stop();
    ::SetWindowPos(HWND(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    myTimer_->start();
}

void MessageWin::ShowUI(const std::string &title_str, const std::string &str)
{
    ShowUI(QString::fromLocal8Bit(title_str.c_str()), QString::fromLocal8Bit(str.c_str()));
}

void MessageWin::SlotTimeout()
{
    this->hide();
}
