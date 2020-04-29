#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDialog>
#include <QDir>

namespace Ui {

class options;
}

struct settings
{
    QString dataDir;
    QString txtDir;
    QString pwDir;
    bool valid=false;
};

extern settings paths;

class options : public QDialog
{
    Q_OBJECT

public:
    explicit options(QWidget *parent = nullptr);
    ~options();

public slots:
    void accept() override;


private slots:
    void on_dataButton_clicked();

    void on_listButton_clicked();

    void on_pwButton_clicked();

    void on_OKButton_clicked();


private:
    Ui::options *ui;
};

#endif // OPTIONS_H
