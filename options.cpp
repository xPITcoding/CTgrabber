#include "options.h"
#include "ui_options.h"
#include "grabber.h"
#include <QFileDialog>
#include <QDir>
#include <QSettings>


settings paths;

options::options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::options)
{
    ui->setupUi(this);

    // QCoreApplication::setOrganizationName("xPit");
    // QCoreApplication::setApplicationName("Data Spy Quantum Version");
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"xPit","Data Spy Quantum Version");
    ::paths.dataDir=settings.value("dataDir","").toString();
    ::paths.txtDir=settings.value("txtDir","").toString();
    ::paths.pwDir=settings.value("pwDir","").toString();
    ::paths.valid = !::paths.dataDir.isEmpty() && !::paths.txtDir.isEmpty() && !::paths.pwDir.isEmpty();

}

void options::accept()
{
    QSettings settings(QSettings::NativeFormat,QSettings::UserScope,"xPit","Data Spy Quantum Version");
    settings.setValue("dataDir",::paths.dataDir);
    settings.setValue("txtDir",::paths.txtDir);
    settings.setValue("pwDir",::paths.pwDir);
    QDialog::accept();

}

options::~options()
{
    delete ui;
}


void options::on_dataButton_clicked()
{
    ::paths.dataDir=QFileDialog::getExistingDirectory(this, tr("select dir"));
    ui->dataEdit->setText(::paths.dataDir);
}


void options::on_listButton_clicked()
{
    ::paths.txtDir=QFileDialog::getExistingDirectory(this, tr("select dir"));
    ui->txtEdit->setText(::paths.txtDir);
}


void options::on_pwButton_clicked()
{
    ::paths.pwDir=QFileDialog::getExistingDirectory(this, tr("select dir"));
    ui->pwEdit->setText(::paths.pwDir);
}


void options::on_OKButton_clicked()
{
    accept();
}


