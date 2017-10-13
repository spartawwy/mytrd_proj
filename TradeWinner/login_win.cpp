#include "login_win.h"

#include <qmessagebox.h>
#include "winner_app.h"

LoginWin::LoginWin(WinnerApp* app)
    : app_(app)
{
    ui_.setupUi(this);

    auto ret = connect(ui_.okButton, SIGNAL(clicked()), this, SLOT(DoOkBtnClicked()));
    ui_.le_pwd->setEchoMode(QLineEdit::Password);
    //setAttribute(Qt::WA_DeleteOnClose);

    ui_.le_name->setText("wwy");
    ui_.le_pwd->setText("123");
}

void LoginWin::closeEvent(QCloseEvent* e)
{
    
}

void LoginWin::Init()
{
    //setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    ////setWindowModality(Qt::ApplicationModal);//设置阻塞类型
    //setAttribute(Qt::WA_ShowModal, true);
}

void LoginWin::DoOkBtnClicked()
{ 
    // todo: read encode use pearid date in sn file
    T_UserInformation user_info;
    auto ret = app_->db_moudle().CheckLogin(ui_.le_name->text().trimmed().toLocal8Bit().data(), ui_.le_pwd->text().trimmed().toLocal8Bit().data()
        , &user_info);

    if( ret < 0 )
    {
        QMessageBox::information(nullptr, "info", QString::fromLocal8Bit("用户名或密码错误!"));

    }else
    {
        app_->user_info(user_info);
        done(QDialog::Accepted);
    }
}
