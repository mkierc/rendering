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

    void setSplitter(int splitter_max);

private slots:
    void on_verticalSliderRotX_valueChanged(int value);
    void on_verticalSliderRotY_valueChanged(int value);
    void on_verticalSliderRotZ_valueChanged(int value);
    void on_verticalSliderZoom_valueChanged(int value);
    void on_horizontalSlider_valueChanged(int value);

    void on_verticalSliderAlpha_valueChanged(int value);

    void on_verticalSliderTreshold_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    void apply_v_transform();
};

extern MainWindow *mainWindow;

#endif // MAINWINDOW_H
