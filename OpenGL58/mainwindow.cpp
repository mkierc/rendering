#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 58 (szum Perlina)");
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


void MainWindow::on_verticalSliderNoise_valueChanged(int value)
{
    ui->widgetOpenGL->set_noise_density(value*0.1);
}


void MainWindow::on_verticalSliderLevel_valueChanged(int value)
{
    ui->widgetOpenGL->set_noise_level(value);
}

void MainWindow::on_verticalSliderZ_valueChanged(int value)
{
    ui->widgetOpenGL->set_noise_z(value*0.01);
}
