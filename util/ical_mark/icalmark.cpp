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
        this->ui->instList->addItem(
            QString((to_string((int)i + 1) + string(". ") + string(annoList[i]))
                        .c_str()));
    }
}
