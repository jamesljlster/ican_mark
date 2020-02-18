#include "icalmark.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ICALMark w;
    w.show();
    return a.exec();
}
