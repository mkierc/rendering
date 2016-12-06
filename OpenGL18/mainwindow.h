#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_verticalSliderRotX_valueChanged(int value);
    void on_verticalSliderRotY_valueChanged(int value);
    void on_verticalSliderRotZ_valueChanged(int value);
    void on_verticalSliderZoom_valueChanged(int value);
    void on_verticalSliderLightX_valueChanged(int value);
    void on_verticalSliderLightY_valueChanged(int value);
    void on_verticalSliderLightZ_valueChanged(int value);

    void on_horizontalSlider_Slider_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    void apply_v_transform();
    void move_light();
};

#endif // MAINWINDOW_H
