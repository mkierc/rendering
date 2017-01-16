#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow *mainWindow = 0;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWindow = this;

    setWindowTitle("OpenGL 55 (wizualizacja wolumetryczna)");
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setSplitter(int splitter_max)
{
    ui->horizontalSlider->setMaximum(splitter_max);
    ui->horizontalSlider->setValue(splitter_max);
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


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->widgetOpenGL->set_slider(value);
}


void MainWindow::on_verticalSliderAlpha_valueChanged(int value)
{
    ui->widgetOpenGL->set_alpha(value*0.001);
}


void MainWindow::on_verticalSliderTreshold_valueChanged(int value)
{
    ui->widgetOpenGL->set_treshold(value*0.001);
}
