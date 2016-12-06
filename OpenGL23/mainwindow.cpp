#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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


void MainWindow::move_light()
{
    ui->widgetOpenGL->move_light(ui->verticalSliderLightX->value()*0.1,
                                 ui->verticalSliderLightY->value()*0.1,
                                 ui->verticalSliderLightZ->value()*0.1);
}

void MainWindow::on_verticalSliderLightX_valueChanged(int value)
{
    move_light();
}

void MainWindow::on_verticalSliderLightY_valueChanged(int value)
{
    move_light();
}

void MainWindow::on_verticalSliderLightZ_valueChanged(int value)
{
    move_light();
}

void MainWindow::on_horizontalSlider_Slider_valueChanged(int value)
{
    ui->widgetOpenGL->move_slider(ui->widgetOpenGL->width()*value/10000);
}
