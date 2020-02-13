#ifndef ICALMARK_H
#define ICALMARK_H

#include <mark_action.hpp>
#include <mark_instance.hpp>
#include <vector>

#include <QListWidgetItem>
#include <QMainWindow>
#include <QModelIndex>

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

    void on_instDel_clicked();

    void on_dataDir_clicked();

    void on_dataRefresh_clicked();

    void on_slideView_currentItemChanged(QListWidgetItem *current,
                                         QListWidgetItem *previous);

    void on_slideNext_clicked();

    void on_slidePrevious_clicked();

    void on_nameFile_clicked();

    void on_nameList_currentIndexChanged(int index);

    void on_instList_currentRowChanged(int currentRow);

    void on_scaleRatio_valueChanged(double arg1);

    void on_scaleRatioSlider_valueChanged(int value);

    void on_imageMap_stateChanged(const QRectF &imageRegion);

   private:
    Ui::ICALMark *ui;

    void slideview_sliding(int step);
    void load_class_names(const QString &filePath);
};
#endif  // ICALMARK_H
