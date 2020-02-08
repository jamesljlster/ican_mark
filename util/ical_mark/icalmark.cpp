#include "icalmark.h"
#include "./ui_icalmark.h"

ICALMark::ICALMark(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ICALMark)
{
    ui->setupUi(this);
}

ICALMark::~ICALMark()
{
    delete ui;
}

