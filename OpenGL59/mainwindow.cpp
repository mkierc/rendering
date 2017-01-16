#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenGL 59 (tekstury proceduralne + szum Perlina)");
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

void MainWindow::on_verticalSliderTexSize_valueChanged(int value)
{
    ui->widgetOpenGL->set_tex_size(value*0.1);
}

void MainWindow::on_verticalSliderTexNoiseFactor_valueChanged(int value)
{
    ui->widgetOpenGL->set_tex_noise_factor(value*0.1);
}

void MainWindow::on_verticalSliderTexNoiseDens_valueChanged(int value)
{
    ui->widgetOpenGL->set_tex_noise_dens(value*0.1);
}

void MainWindow::on_verticalSliderTexPow_valueChanged(int value)
{
    ui->widgetOpenGL->set_tex_pow(value*0.1);
}

void MainWindow::on_pushButtonMarble_clicked()
{
    ui->verticalSliderTexSize->setValue(137);
    ui->verticalSliderTexNoiseDens->setValue(21);
    ui->verticalSliderTexNoiseFactor->setValue(66);
    ui->verticalSliderTexPow->setValue(150);
    ui->verticalSliderTexNoiseFractal->setValue(5);
    ui->horizontalSlider_Slider->setValue(100000);
}

void MainWindow::on_pushButtonWood_clicked()
{
    ui->verticalSliderTexSize->setValue(143);
    ui->verticalSliderTexNoiseDens->setValue(35);
    ui->verticalSliderTexNoiseFactor->setValue(4);
    ui->verticalSliderTexPow->setValue(17);
    ui->verticalSliderTexNoiseFractal->setValue(1);
    ui->horizontalSlider_Slider->setValue(0);
}

void MainWindow::on_verticalSliderTexNoiseFractal_valueChanged(int value)
{
    ui->widgetOpenGL->set_tex_noise_spect(value);
}
