#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 45 (teren)");
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


void MainWindow::on_checkBoxLines_toggled(bool checked)
{
    ui->widgetOpenGL->set_lines(checked);
}

