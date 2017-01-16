#include <QDateTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 47 (glebia ostrosci)");
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


void MainWindow::on_verticalSliderGamma_valueChanged(int value)
{
    float gamma = float(value)*0.01;
    ui->label_gamma->setText(QString("%1").arg(gamma, 0, 'f', 2));
    ui->widgetOpenGL->set_gamma(gamma);
}


void MainWindow::on_verticalSliderLightB_valueChanged(int value)
{
    ui->widgetOpenGL->set_light_brightness(float(value)*0.1);
}


void MainWindow::on_verticalSliderDistance_valueChanged(int value)
{
    ui->widgetOpenGL->set_focus(ui->verticalSliderDistance->value()*0.01, ui->verticalSliderRange->value()*0.01);
}


void MainWindow::on_verticalSliderRange_valueChanged(int value)
{
    on_verticalSliderDistance_valueChanged(value);
}
