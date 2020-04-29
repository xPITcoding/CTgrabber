#ifndef ADDUSR_H
#define ADDUSR_H

#include <QDialog>

namespace Ui {
class addusr;
}

class addusr : public QDialog
{
    Q_OBJECT

public:
    explicit addusr(QWidget *parent = nullptr);
    ~addusr();

    QString user(){return newusr;};
    QString pw(){return newpw;}

public slots:
    virtual void accept() override;

private:
   Ui::addusr *ui;

   QString newusr;
   QString newpw;

};

#endif // ADDUSR_H
