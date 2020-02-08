#ifndef ICALMARK_H
#define ICALMARK_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ICALMark; }
QT_END_NAMESPACE

class ICALMark : public QMainWindow
{
    Q_OBJECT

public:
    ICALMark(QWidget *parent = nullptr);
    ~ICALMark();

private:
    Ui::ICALMark *ui;
};
#endif // ICALMARK_H
