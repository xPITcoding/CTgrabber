#ifndef LOGIN_H
#define LOGIN_H
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include <windows.h>
#include <winuser.h>

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT


public:
    explicit login(QWidget *parent = nullptr);
    ~login();

    QString getusr(){return currentusr;};
    bool loggedIn=false;

private slots:
    void on_addusrButton_clicked();
    void load();
    void save();

    void on_loginButton_clicked();


    void hideWin();

private:
    Ui::login *ui;
    QMap<QString,QString> pwBase;
    QTimer * timer;

    HWND QuWihandle;

protected:
    virtual void keyPressEvent(QKeyEvent *) override;
    virtual void closeEvent(QCloseEvent *) override;

    QString currentusr;

};

QString encrypt(QString);
QString decrypt(QString);

#endif // LOGIN_H
