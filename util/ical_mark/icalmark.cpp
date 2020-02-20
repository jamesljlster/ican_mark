#include "icalmark.h"
#include "./ui_icalmark.h"

#include <fstream>
#include <iostream>
#include <string>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>
#include <QToolButton>

#define MARK_EXT ".mark"

using namespace std;
using namespace ical_mark;

ICALMark::ICALMark(QWidget* parent) : QMainWindow(parent), ui(new Ui::ICALMark)
{
    ui->setupUi(this);

    // Setup timer
    this->ctrlTimer = new QTimer();
    connect(this->ctrlTimer, &QTimer::timeout, this,
            &ICALMark::ctrl_timer_event);
    this->ctrlTimer->start(1000 / this->ui->moveSpeed->value());

    // Connect signals and slots
    connect(this->ui->markArea, &RBoxMarkWidget::imageRegionChanged,
            this->ui->imageMap, &ImageMap::set_select_region);
    connect(this->ui->imageMap, &ImageMap::selectRegionChanged,
            this->ui->markArea, &RBoxMarkWidget::set_image_region);

    connect(this->ui->nameList,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this->ui->markArea, &RBoxMarkWidget::set_mark_label);
    connect(this->ui->instList, &QListWidget::currentRowChanged,
            this->ui->markArea, &RBoxMarkWidget::set_hl_instance_index);

    connect(this->ui->scaleRatio,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this->ui->markArea, &RBoxMarkWidget::set_scale_ratio);

    // Setup south tab widget controller
    this->setup_tab_controller();
}

ICALMark::~ICALMark() { delete ui; }

void ICALMark::setup_tab_controller()
{
    // Setup tab controll buttons
    QButtonGroup* btnGroup = new QButtonGroup(this);
    QStackedWidget* root = this->ui->southTab;
    for (int i = 0; i < root->count(); i++)
    {
        // Create and set new button
        QToolButton* btn = new QToolButton(this);
        btn->setCheckable(true);
        btn->setText(root->widget(i)->accessibleName());

        // Add button to group and UI
        btnGroup->addButton(btn, i);
        this->ui->sTabCtrl->addWidget(btn);
    }

    // Add spacer to button group
    this->ui->sTabCtrl->addStretch();

    // Setup controller initial condition
    this->ui->southTab->setCurrentIndex(0);
    btnGroup->button(0)->setChecked(true);
    btnGroup->setExclusive(true);

    // Setup signal and slot
    connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            [=](int id) {
                QStackedWidget* root = this->ui->southTab;
                int tabIndex = root->currentIndex();
                if (tabIndex == id)
                {
                    // Hide or show tab widget
                    bool visible = !root->isVisible();
                    root->setVisible(visible);

                    // Set button check state
                    btnGroup->setExclusive(false);
                    btnGroup->button(id)->setChecked(visible);
                    btnGroup->setExclusive(true);
                }
                else
                {
                    root->setCurrentIndex(id);
                    root->show();
                }
            });
}

void ICALMark::on_markArea_instanceListChanged(const vector<Instance>& annoList)
{
    // Refresh marked instances list
    this->ui->instList->clear();
    for (size_t i = 0; i < annoList.size(); i++)
    {
        // Generate string representation of item
        string itemStr = to_string((int)i + 1) + string(". ");
        if (annoList[i].label < this->ui->nameList->count())
        {
            itemStr +=
                this->ui->nameList->itemText(annoList[i].label).toStdString() +
                string(" ");
        }

        itemStr += string(annoList[i]);

        // Add item to list
        QListWidgetItem* item = new QListWidgetItem(itemStr.c_str());

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);

        this->ui->instList->addItem(item);
    }

    // Save data
    QString filePath =
        QFileInfo(this->ui->dataDir->text(),
                  this->ui->slideView->currentItem()->text() + MARK_EXT)
            .filePath();

    YAML::Node node;
    node = annoList;

    YAML::Emitter out;
    out << node;

    std::ofstream fWriter(filePath.toStdString());
    if (!fWriter.is_open())
    {
        QMessageBox::warning(
            this, QString(tr("Error")),
            QString(tr("Failed to write file:")) + QString("\n") + filePath);
    }

    fWriter << out.c_str();
    fWriter.close();

    // Change sample marked state
    this->ui->slideView->currentItem()->setCheckState(Qt::CheckState::Checked);
}

void ICALMark::on_markArea_scaleRatioChanged(qreal ratio)
{
    if (ratio > this->ui->scaleRatio->maximum())
    {
        this->ui->scaleRatio->setMaximum(ratio);
    }

    this->ui->scaleRatio->setValue(ratio);
}

void ICALMark::on_instDel_clicked()
{
    vector<size_t> indList;
    for (int i = 0; i < this->ui->instList->count(); i++)
    {
        if (this->ui->instList->item(i)->checkState() == Qt::Checked)
        {
            indList.push_back(i);
        }
    }

    this->ui->markArea->delete_instances(indList);
}

void ICALMark::on_dataDir_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);

    if (dialog.exec())
    {
        QString pathStr = dialog.selectedFiles()[0];
        this->ui->dataDir->setText(pathStr);

        this->on_dataRefresh_clicked();
    }
}

void ICALMark::on_dataRefresh_clicked()
{
    // Set name filter
    QList<QByteArray> fmtList = QImageReader::supportedImageFormats();
    QStringList filter;
    for (const QByteArray& fmt : fmtList)
    {
        filter << QString("*.") + QString(fmt);
    }

    // Scan images
    QDir dir = QDir(this->ui->dataDir->text());
    QFileInfoList fileList = dir.entryInfoList(filter);
    QList<QListWidgetItem*> unmarkedList;
    QList<QListWidgetItem*> markedList;

    this->ui->slideView->clear();
    for (const QFileInfo& fileInfo : fileList)
    {
        QListWidgetItem* item = new QListWidgetItem(
            QIcon(fileInfo.absoluteFilePath()), fileInfo.fileName());
        item->setFlags(item->flags() &= ~Qt::ItemIsUserCheckable);
        if (QFileInfo::exists(fileInfo.absoluteFilePath() + MARK_EXT))
        {
            item->setCheckState(Qt::Checked);
            markedList.append(item);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
            unmarkedList.append(item);
        }
    }

    for (QListWidgetItem* item : unmarkedList)
    {
        this->ui->slideView->addItem(item);
    }

    for (QListWidgetItem* item : markedList)
    {
        this->ui->slideView->addItem(item);
    }

    // Auto looking for class names
    filter.clear();
    filter.push_back("*.names");
    fileList = dir.entryInfoList(filter);
    if (fileList.count())
    {
        this->load_class_names(fileList[0].absoluteFilePath());
    }
}

void ICALMark::on_slideView_currentItemChanged(QListWidgetItem* current,
                                               QListWidgetItem* previous)
{
    (void)previous;

    if (current)
    {
        vector<Instance> instList;

        // Get image path
        QString imgPath =
            QFileInfo(this->ui->dataDir->text(), current->text()).filePath();

        // Load marked instances
        if (current->checkState() == Qt::Checked)
        {
            try
            {
                YAML::Node node =
                    YAML::LoadFile((imgPath + MARK_EXT).toStdString());
                instList = node.as<vector<Instance>>();
            }
            catch (exception& ex)
            {
                QMessageBox::warning(
                    this, QString(tr("Error")),
                    QString(tr("Failed to load marked information")) +
                        QString("\n") + QString(ex.what()));
            }
        }

        // Load images and reset mark area
        QImage image = QImage(imgPath);
        this->ui->imageMap->reset(image);
        this->ui->mapStack->setCurrentIndex(0);

        this->ui->markArea->reset(image, instList);
        this->ui->markStack->setCurrentIndex(0);
    }
    else
    {
        // Disable mark area and image map
        this->ui->mapStack->setCurrentIndex(1);
        this->ui->markStack->setCurrentIndex(1);

        // Clear instances list
        this->ui->instList->clear();
    }
}

void ICALMark::ctrl_timer_event()
{
    // Image region moving
    int dy = (this->s - this->w) * this->moveStep;
    int dx = (this->d - this->a) * this->moveStep;
    if (dx || dy)
    {
        this->ui->markArea->move_image_region(dx, dy);
    }
}

void ICALMark::slideview_sliding(int step)
{
    QModelIndex curInd = this->ui->slideView->currentIndex();
    QModelIndex nextInd = curInd.sibling(curInd.row() + step, 0);
    if (nextInd.isValid())
    {
        this->ui->slideView->setCurrentIndex(nextInd);
        this->ui->slideView->scrollTo(
            nextInd, QAbstractItemView::ScrollHint::PositionAtCenter);
    }
}

void ICALMark::on_slideNext_clicked()
{
    if (this->ui->slideView->currentIndex().isValid())
    {
        this->slideview_sliding(1);
    }
    else
    {
        QAbstractItemModel* model = this->ui->slideView->model();
        this->ui->slideView->setCurrentIndex(model->index(0, 0));
    }
}

void ICALMark::on_slidePrevious_clicked()
{
    if (this->ui->slideView->currentIndex().isValid())
    {
        this->slideview_sliding(-1);
    }
    else
    {
        QAbstractItemModel* model = this->ui->slideView->model();
        this->ui->slideView->setCurrentIndex(
            model->index(model->rowCount() - 1, model->columnCount() - 1));
    }
}

void ICALMark::load_class_names(const QString& filePath)
{
    try
    {
        // Load names
        YAML::Node node = YAML::LoadFile(filePath.toStdString());
        vector<string> classNames = node.as<vector<string>>();
        this->ui->markArea->set_class_names(classNames);

        // Apply to name list
        this->ui->nameList->clear();
        for (const string& nameStr : classNames)
        {
            this->ui->nameList->addItem(QString(nameStr.c_str()));
        }

        // Reset mark label
        this->ui->markArea->set_mark_label(0);
    }
    catch (exception& ex)
    {
        QMessageBox::warning(this, QString(tr("Error")),
                             QString(tr("Failed to load names file")) +
                                 QString("\n") + QString(ex.what()));
    }
}

void ICALMark::on_nameFile_clicked()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);

    if (dialog.exec())
    {
        this->load_class_names(dialog.selectedFiles()[0]);
    }
}

void ICALMark::on_scaleRatio_valueChanged(double arg1)
{
    int val = arg1 * 10;
    if (val > this->ui->scaleRatioSlider->maximum())
    {
        this->ui->scaleRatioSlider->setMaximum(val);
    }

    this->ui->scaleRatioSlider->setValue(val);
}

void ICALMark::on_scaleRatioSlider_valueChanged(int value)
{
    this->ui->scaleRatio->setValue((double)value / 10);
}

void ICALMark::keyPressEvent(QKeyEvent* event)
{
    // Key shortcuts for moving image region
    if (this->ui->markStack->currentIndex() == 0)
    {
        int key = event->key();
        this->w = (key == Qt::Key_W) ? 1 : this->w;
        this->a = (key == Qt::Key_A) ? 1 : this->a;
        this->s = (key == Qt::Key_S) ? 1 : this->s;
        this->d = (key == Qt::Key_D) ? 1 : this->d;
    }

    if (event->isAutoRepeat())
    {
        return;
    }

    // Key shortcurs for resetting focus
    if (event->key() == Qt::Key_Escape)
    {
        this->ui->markArea->setFocus();
    }

    // Key shortcuts for marking action
    if (this->ui->markStack->currentIndex() == 0)
    {
        switch (event->key())
        {
            case Qt::Key_R:
                this->ui->markArea->marking_revert();
                break;

            case Qt::Key_Backspace:
                this->ui->markArea->marking_reset();
                break;
        }
    }

    // Key shortcuts for label switching
    int newLabelIndex = this->ui->nameList->currentIndex();
    switch (event->key())
    {
        case Qt::Key_Up:
            newLabelIndex = newLabelIndex - 1;
            if (newLabelIndex >= 0)
            {
                this->ui->nameList->setCurrentIndex(newLabelIndex);
            }

            break;

        case Qt::Key_Down:
            newLabelIndex = newLabelIndex + 1;
            if (newLabelIndex < this->ui->nameList->count())
            {
                this->ui->nameList->setCurrentIndex(newLabelIndex);
            }

            break;
    }

    // Key shortcuts for sample sliding
    switch (event->key())
    {
        case Qt::Key_Right:
        case Qt::Key_Space:
            this->ui->slideNext->animateClick();
            break;

        case Qt::Key_Left:
            this->ui->slidePrevious->animateClick();
            break;
    }
}

void ICALMark::keyReleaseEvent(QKeyEvent* event)
{
    // Key shortcuts for moving image region
    int key = event->key();
    this->w = (key == Qt::Key_W) ? 0 : this->w;
    this->a = (key == Qt::Key_A) ? 0 : this->a;
    this->s = (key == Qt::Key_S) ? 0 : this->s;
    this->d = (key == Qt::Key_D) ? 0 : this->d;
}

void ICALMark::on_moveSpeed_valueChanged(int arg1)
{
    this->ctrlTimer->start(1000 / arg1);
}
