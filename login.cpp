#include "login.h"
#include "ui_login.h"
#include <QComboBox>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QMessageBox>
#include "addusr.h"
#include <QKeyEvent>
#include <QCloseEvent>
#include "options.h"
#include <windows.h>
#include <winuser.h>
#include <QTimer>


login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    setWindowTitle( tr("User Login") );
    setModal( true );
    setWindowFlags(Qt::FramelessWindowHint);


    LPCSTR lpClassName="WindowsForms10.Window.8.app.0.378734a";
    LPCSTR lpWindowName=0;

    QuWihandle = FindWindowA(lpClassName, lpWindowName);

    //QMessageBox::critical(0,"","handle"+ QString ("0x%1").arg((long)QuWihandle,0,16));


    timer = new QTimer();
    timer->setInterval(1000);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(hideWin()));
    timer->start();

    load();

    pwBase["admin"]="ct2admin2020";

    //show logo
    QPixmap aPixmap(":/img/logo.png");
    aPixmap = aPixmap.scaledToWidth(191);
    ui->logolabel->setPixmap(aPixmap);

    for (QMap<QString,QString>::iterator it=pwBase.begin(); it!=pwBase.end(); it++)
        ui->usrcomboBox->addItem(it.key());

}

void login::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
}

void login::closeEvent(QCloseEvent * ce)
{
    ce->ignore();
}

void login::save()
{
    QFile f(::paths.pwDir + "\\pwlist.txt");
    if (f.open(QFile::WriteOnly))
    {
        QTextStream t(&f);
        t << pwBase.count() << endl;

        for (QMap<QString,QString>::iterator it=pwBase.begin(); it!=pwBase.end(); it++)
        {
            t << it.key() << "\t" << encrypt(*it) << endl;
        }
        f.close();
    }
}

void login::load()
{
    QFile f(::paths.pwDir + "\\pwlist.txt");
    if (f.open(QFile::ReadOnly))
    {
        QTextStream t(&f);
        pwBase.clear();
        long nr;
        nr = t.readLine().toLong();
        for (long i=0; i<nr; i++)
        {
            QString s = t.readLine();
            pwBase[s.section("\t",0,0)] = decrypt(s.section("\t",1,1));
        }
    }

}





login::~login()
{
    save();
    delete ui;
}

void login::on_addusrButton_clicked()
{
    addusr dlg(0);
    if (dlg.exec() == QDialog::Accepted)
    {
        if (!pwBase.contains(dlg.user()))
           {
            pwBase[dlg.user()] = dlg.pw();
            ui->usrcomboBox->addItem(dlg.user());
            ui->usrcomboBox->setCurrentText(dlg.user());
           }
        else
            QMessageBox::critical(0,"ERROR","user already exists!");

    }
}

void login::on_loginButton_clicked()
{
    QString loginname = ui->usrcomboBox->currentText();
    QString password = ui->pwEdit->text();

    if (pwBase.contains(loginname) && pwBase[loginname] == password)
    {
        currentusr=loginname;
        timer->stop();
        loggedIn=true;
        hideWin();
        accept();
    }
    else
        QMessageBox::critical(0, " WARNING ", "no matching user / password combination!");
}

QString encrypt(QString in)
{
    QString out;

    for (int i=0; i<in.length(); ++i)
    {
        out.append(QChar::fromLatin1(255-in[i].toLatin1()));
    }
    return out;
}

QString decrypt(QString in)
{
    QString out;

    for (int i=0; i<in.length(); ++i)
    {
        out.append(QChar::fromLatin1(255-in[i].toLatin1()));
    }
    return out;
}


void login::hideWin()
{

    if(!loggedIn)
    {
        EnableWindow(QuWihandle,false);
        //QMessageBox::critical(0,"","MINIMIEREN!"+ QString ("0x%1").arg((long)QuWihandle,0,16));
    }

    else
    {
        EnableWindow(QuWihandle,true);
        //QMessageBox::critical(0,"","ANZEIGEN"+ QString ("0x%1").arg((long)QuWihandle,0,16));
    }
}
