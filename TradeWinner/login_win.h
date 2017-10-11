#ifndef LOGIN_H_SDF23DSFKKSDF
#define LOGIN_H_SDF23DSFKKSDF

#include <QtWidgets/QDialog>

#include "ui_login_dlg.h"

class WinnerApp;
class LoginWin : public QDialog
{
    Q_OBJECT

public:

    LoginWin(WinnerApp* app);

    virtual void closeEvent(QCloseEvent* e) override;
    
    void Init();
    
public slots:
        void DoOkBtnClicked();

private:

    Ui::login_dlg  ui_;
    WinnerApp *app_;
};

#endif