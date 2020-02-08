#ifndef ICALMARK_H
#define ICALMARK_H

#include <mark_action.hpp>
#include <mark_instance.hpp>
#include <vector>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ICALMark;
}
QT_END_NAMESPACE

class ICALMark : public QMainWindow
{
    Q_OBJECT

   public:
    ICALMark(QWidget *parent = nullptr);
    ~ICALMark();

   private slots:
    void on_markArea_stateChanged(
        const std::vector<ical_mark::Instance> &annoList);

   private:
    Ui::ICALMark *ui;
};
#endif  // ICALMARK_H
