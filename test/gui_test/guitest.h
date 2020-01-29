#ifndef GUITEST_H
#define GUITEST_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class GuiTest; }
QT_END_NAMESPACE

class GuiTest : public QMainWindow
{
    Q_OBJECT

public:
    GuiTest(QWidget *parent = nullptr);
    ~GuiTest();

private:
    Ui::GuiTest *ui;
};
#endif // GUITEST_H
