#ifndef ICALMARK_H
#define ICALMARK_H

#include <mark_action.hpp>
#include <mark_instance.hpp>
#include <vector>

#include <QListWidgetItem>
#include <QMainWindow>
#include <QModelIndex>
#include <QTimer>

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
    ICALMark(QWidget* parent = nullptr);
    ~ICALMark();

   private slots:
    void on_markArea_instanceListChanged(
        const std::vector<ical_mark::Instance>& annoList);

    void on_markArea_scaleRatioChanged(qreal ratio);

    void on_instDel_clicked();

    void on_dataDir_clicked();

    void on_dataRefresh_clicked();

    void on_slideView_currentItemChanged(QListWidgetItem* current,
                                         QListWidgetItem* previous);

    void on_slideNext_clicked();

    void on_slidePrevious_clicked();

    void on_nameFile_clicked();

    void on_scaleRatio_valueChanged(double arg1);

    void on_scaleRatioSlider_valueChanged(int value);

    void ctrl_timer_event();

    void on_moveSpeed_valueChanged(int arg1);

   private:
    Ui::ICALMark* ui;
    QTimer* ctrlTimer;

    // For moving image region
    int moveStep = 1;
    int w = 0, a = 0, s = 0, d = 0;

    void setup_tab_controller();

    void slideview_sliding(int step);
    void load_class_names(const QString& filePath);

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};
#endif  // ICALMARK_H
