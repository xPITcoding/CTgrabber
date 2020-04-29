#include "addusr.h"
#include "ui_addusr.h"
#include <QMessageBox>


addusr::addusr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addusr)
{
    ui->setupUi(this);
}

addusr::~addusr()
{
    delete ui;
}


void addusr::accept()
{
    QString login = ui->newusrEdit->text();
    QString pw1 = ui->newpwEdit->text();
    QString pw2 = ui->newpw2Edit->text();

    if (pw1 == pw2)
    {
        newusr = login;
        newpw = pw1;
        QDialog::accept();
    }

    else
        QMessageBox::critical(0,"ERROR","passwords do not match!");
}
