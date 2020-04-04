#ifndef ICANMARK_H
#define ICANMARK_H

#include <mark_action.hpp>
#include <mark_instance.hpp>
#include <vector>

#include <QListWidgetItem>
#include <QMainWindow>
#include <QModelIndex>
#include <QPointer>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
class ICANMark;
}
QT_END_NAMESPACE

class ICANMark : public QMainWindow
{
    Q_OBJECT

   public:
    ICANMark(QWidget* parent = nullptr);
    ~ICANMark();

   private slots:
    void ctrl_timer_event();
    void setup_move_timer(int fps);

    void on_markArea_instanceListChanged(
        const std::vector<ican_mark::Instance>& annoList);

    void on_instDel_clicked();

    void on_dataDir_clicked();

    void on_dataRefresh_clicked();

    void on_slideView_currentItemChanged(QListWidgetItem* current,
                                         QListWidgetItem* previous);

    void on_slideNext_clicked();

    void on_slidePrevious_clicked();

    void on_nameFile_clicked();

    void on_scaleToFit_clicked();

   private:
    Ui::ICANMark* ui;
    QPointer<QTimer> ctrlTimer;

    // For moving image region
    int updateInterval;
    int moveStep = 1;
    int w = 0, a = 0, s = 0, d = 0;

    void setup_tab_controller();

    void slideview_sliding(int step);
    void load_class_names(const QString& filePath);
    void label_switching(int step);

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};
#endif  // ICANMARK_H
