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
    void on_verticalSliderTerHeightScale_valueChanged(int value);
    void on_verticalSliderTerHeightOffset_valueChanged(int value);
    void on_checkBoxLines_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    void apply_v_transform();
};

#endif // MAINWINDOW_H
