#include "icalmark.h"
#include "./ui_icalmark.h"

#include <fstream>
#include <iostream>
#include <string>

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>

#define MARK_EXT ".mark"

using namespace std;
using namespace ical_mark;

ICALMark::ICALMark(QWidget* parent) : QMainWindow(parent), ui(new Ui::ICALMark)
{
    ui->setupUi(this);

    // Set default dataset folder
    this->ui->dataDir->setText(
        QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]);
}

ICALMark::~ICALMark() { delete ui; }

void ICALMark::on_markArea_stateChanged(const vector<Instance>& annoList)
{
    // Refresh marked instances list
    this->ui->instList->clear();
    for (size_t i = 0; i < annoList.size(); i++)
    {
        string itemStr =
            to_string((int)i + 1) + string(". ") + string(annoList[i]);
        QListWidgetItem* item = new QListWidgetItem(itemStr.c_str());

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);

        this->ui->instList->addItem(item);
    }

    // File and state processing
    QFileInfo fileInfo =
        QFileInfo(this->ui->dataDir->text(),
                  this->ui->slideView->currentItem()->text() + MARK_EXT);
    if (annoList.size())
    {
        // Save data
        YAML::Node node;
        node = annoList;

        YAML::Emitter out;
        out << node;

        QString filePath = fileInfo.filePath();
        std::ofstream fWriter(filePath.toStdString());
        if (!fWriter.is_open())
        {
            QMessageBox::warning(this, QString(tr("Error")),
                                 QString(tr("Failed to write file:")) +
                                     QString("\n") + filePath);
        }

        fWriter << out.c_str();
        fWriter.close();

        // Change state
        this->ui->slideView->currentItem()->setCheckState(
            Qt::CheckState::Checked);
    }
    else
    {
        // Remove data
        if (fileInfo.exists())
        {
            QFile::remove(fileInfo.filePath());
        }
    }
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
    }

    this->on_dataRefresh_clicked();
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

    this->ui->slideView->clear();
    for (const QFileInfo& fileInfo : fileList)
    {
        QListWidgetItem* item = new QListWidgetItem(
            QIcon(fileInfo.absoluteFilePath()), fileInfo.fileName());
        item->setFlags(item->flags() &= ~Qt::ItemIsUserCheckable);
        if (QFileInfo::exists(fileInfo.absoluteFilePath() + MARK_EXT))
        {
            item->setCheckState(Qt::Checked);
        }
        else
        {
            item->setCheckState(Qt::Unchecked);
        }

        this->ui->slideView->addItem(item);
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
                    YAML::Load((imgPath + MARK_EXT).toStdString());
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
        this->ui->markArea->reset(image, instList);
        this->ui->markStack->setCurrentIndex(0);
    }
    else
    {
        // Disable mark area
        this->ui->markStack->setCurrentIndex(1);
    }
}
