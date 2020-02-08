#include "icalmark.h"
#include "./ui_icalmark.h"

#include <string>

#include <QCheckBox>
#include <QString>

using namespace std;
using namespace ical_mark;

ICALMark::ICALMark(QWidget* parent) : QMainWindow(parent), ui(new Ui::ICALMark)
{
    ui->setupUi(this);
}

ICALMark::~ICALMark() { delete ui; }

void ICALMark::on_markArea_stateChanged(const vector<Instance>& annoList)
{
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
