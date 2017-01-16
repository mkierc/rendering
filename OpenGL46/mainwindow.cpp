#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 46 (mgla)");
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


void MainWindow::on_verticalSliderTerHeightScale_valueChanged(int value)
{
    ui->widgetOpenGL->m_transform(ui->verticalSliderTerHeightScale->value()*0.1,
                                  (ui->verticalSliderTerHeightOffset->value()- 100)*0.05);
}


void MainWindow::on_verticalSliderTerHeightOffset_valueChanged(int value)
{
  on_verticalSliderTerHeightScale_valueChanged(value);
}


void MainWindow::on_horizontalSlider_Slider_valueChanged(int value)
{
    ui->widgetOpenGL->move_slider(ui->widgetOpenGL->width()*value/10000);
}


void MainWindow::on_verticalSliderFogBrightness_valueChanged(int value)
{
    ui->widgetOpenGL->set_fog(ui->verticalSliderFogBrightness->value()*0.001,
                              ui->verticalSliderFogDensity->value()*0.001,
                              ui->verticalSliderFogDensity2->value()*0.001,
                              ui->verticalSliderFogLevel->value()*0.001);
}


void MainWindow::on_verticalSliderFogDensity_valueChanged(int value)
{
    on_verticalSliderFogBrightness_valueChanged(value);
}


void MainWindow::on_verticalSliderFogDensity2_valueChanged(int value)
{
    on_verticalSliderFogBrightness_valueChanged(value);
}


void MainWindow::on_verticalSliderFogLevel_valueChanged(int value)
{
    on_verticalSliderFogBrightness_valueChanged(value);
}
