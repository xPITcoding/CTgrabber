#include "grabber.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Grabber w;
    w.show();
    return a.exec();
}


