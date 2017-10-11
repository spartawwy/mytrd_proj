
#include <qdebug.h>
#include <QListWidget>

class QLineEdit;
class HintList: public QListWidget
{
    Q_OBJECT
public:
    HintList(QWidget *parent, QLineEdit *le_stock);
    ~HintList();

    void HintList::ShowUI();

    virtual void keyPressEvent(QKeyEvent *event) override;

private slots:

    void checkSlots();

signals:

    void choiceStr(QString);

private:

    QTimer  *myTimer;
    QLineEdit *le_stock_;
};