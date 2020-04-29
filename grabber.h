#ifndef GRABBER_H
#define GRABBER_H

#include <QWidget>
#include <QDateTime>
#include <QTextStream>
#include <QSystemTrayIcon>
#include <QTime>
#include <QChartView>
#include "options.h"
#include <QShortcut>

using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui { class Grabber; }
QT_END_NAMESPACE

class fileInfo
{
    public:
    QString bname;
    QString user;
    QString pname;
    QDate created;
    QTime ctime;
    float fsize;
    float scantime;
    int kV;
    int uA;
    int FOV;

    void save(QTextStream & );
    QString toString ();

    fileInfo(QTextStream & );
    fileInfo(){};
};


class Grabber : public QWidget
{
    Q_OBJECT

public slots:

    void searchDir();




public:
    Grabber(QWidget *parent = nullptr);
    ~Grabber();

    QDate plotDate;
    QShortcut *shortcut;

protected slots:
    void load();
    void save();
    void plot();

private slots:


    void on_minButton_clicked();

    void on_setButton_clicked();

    void on_plotButton_clicked();
    
    void on_exportButton_clicked();

    void on_logoutButton_clicked();

    void on_optionsButton_clicked();

    void on_dateEdit_userDateChanged(const QDate &date);

private:
    Ui::Grabber *ui;
    QMap<QString,fileInfo> dataBase;
    QSystemTrayIcon * sysIcon;
    QTimer * timer;
    QChartView * motimeview;
    QChartView * momemoryview;
    QChartView * yetimeview;
    QChartView * yememoryview;
    QChartView * umotimeview;
    QChartView * umomemoryview;
    QChartView * uyetimeview;
    QChartView * uyememoryview;

    options * optionDlg;

protected:
    virtual void closeEvent(QCloseEvent *) override;
    virtual void keyPressEvent(QKeyEvent *) override;
};
#endif // GRABBER_H
