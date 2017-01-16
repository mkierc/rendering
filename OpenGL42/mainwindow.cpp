#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 42");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::apply_v_transform()
{
    ui->widgetOpenGL->v_transform(ui->verticalSliderRotX->value()*0.1,
                                  ui->verticalSliderRotY->value()*0.1,
                                  ui->verticalSliderRotZ->value()*0.1,
                                  ui->verticalSliderZoom->value()*0.01);
}


void MainWindow::on_verticalSliderRotX_valueChanged(int value)
{
    apply_v_transform();
}

void MainWindow::on_verticalSliderRotY_valueChanged(int value)
{
    apply_v_transform();
}

void MainWindow::on_verticalSliderRotZ_valueChanged(int value)
{
    apply_v_transform();
}

void MainWindow::on_verticalSliderZoom_valueChanged(int value)
{
    apply_v_transform();
}

void MainWindow::on_verticalSliderInner_valueChanged(int value)
{
    ui->widgetOpenGL->set_tess(ui->verticalSliderInner->value(),
                               ui->verticalSliderOuter0->value(),
                               ui->verticalSliderOuter1->value(),
                               ui->verticalSliderOuter2->value());
}

void MainWindow::on_verticalSliderOuter0_valueChanged(int value)
{
    ui->widgetOpenGL->set_tess(ui->verticalSliderInner->value(),
                               ui->verticalSliderOuter0->value(),
                               ui->verticalSliderOuter1->value(),
                               ui->verticalSliderOuter2->value());
}

void MainWindow::on_verticalSliderOuter1_valueChanged(int value)
{
    ui->widgetOpenGL->set_tess(ui->verticalSliderInner->value(),
                               ui->verticalSliderOuter0->value(),
                               ui->verticalSliderOuter1->value(),
                               ui->verticalSliderOuter2->value());
}

void MainWindow::on_verticalSliderOuter2_valueChanged(int value)
{
    ui->widgetOpenGL->set_tess(ui->verticalSliderInner->value(),
                               ui->verticalSliderOuter0->value(),
                               ui->verticalSliderOuter1->value(),
                               ui->verticalSliderOuter2->value());
}
