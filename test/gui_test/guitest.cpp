#include "guitest.h"
#include "./ui_guitest.h"

GuiTest::GuiTest(QWidget *parent) : QMainWindow(parent), ui(new Ui::GuiTest)
{
    ui->setupUi(this);
}

GuiTest::~GuiTest() { delete ui; }

void GuiTest::on_horizontalSlider_valueChanged(int value)
{
    qreal ratio = (qreal)value / 10.0;
    this->ui->widget->set_ratio(ratio);
    this->ui->doubleSpinBox->setValue(ratio);
}

void GuiTest::on_doubleSpinBox_valueChanged(double arg1)
{
    this->ui->widget->set_ratio(arg1);
    this->ui->horizontalSlider->setValue(arg1 * 10);
}
