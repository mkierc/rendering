/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>
#include <widgetopengl.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    WidgetOpenGL *widgetOpenGL;
    QSlider *horizontalSlider_Slider;
    QGroupBox *groupBoxView;
    QHBoxLayout *horizontalLayout;
    QSlider *verticalSliderRotX;
    QSlider *verticalSliderRotZ;
    QSlider *verticalSliderRotY;
    QSpacerItem *horizontalSpacer;
    QSlider *verticalSliderZoom;
    QGroupBox *groupBoxLight;
    QHBoxLayout *horizontalLayout_2;
    QSlider *verticalSliderLightX;
    QSlider *verticalSliderLightY;
    QSlider *verticalSliderLightZ;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(883, 595);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        widgetOpenGL = new WidgetOpenGL(centralWidget);
        widgetOpenGL->setObjectName(QStringLiteral("widgetOpenGL"));
        widgetOpenGL->setMinimumSize(QSize(200, 200));

        gridLayout->addWidget(widgetOpenGL, 0, 0, 1, 1);

        horizontalSlider_Slider = new QSlider(centralWidget);
        horizontalSlider_Slider->setObjectName(QStringLiteral("horizontalSlider_Slider"));
        horizontalSlider_Slider->setMaximum(10000);
        horizontalSlider_Slider->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Slider, 1, 0, 1, 1);

        groupBoxView = new QGroupBox(centralWidget);
        groupBoxView->setObjectName(QStringLiteral("groupBoxView"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBoxView->sizePolicy().hasHeightForWidth());
        groupBoxView->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(groupBoxView);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalSliderRotX = new QSlider(groupBoxView);
        verticalSliderRotX->setObjectName(QStringLiteral("verticalSliderRotX"));
        verticalSliderRotX->setMaximum(3600);
        verticalSliderRotX->setValue(200);
        verticalSliderRotX->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(verticalSliderRotX);

        verticalSliderRotZ = new QSlider(groupBoxView);
        verticalSliderRotZ->setObjectName(QStringLiteral("verticalSliderRotZ"));
        verticalSliderRotZ->setMaximum(3600);
        verticalSliderRotZ->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(verticalSliderRotZ);

        verticalSliderRotY = new QSlider(groupBoxView);
        verticalSliderRotY->setObjectName(QStringLiteral("verticalSliderRotY"));
        verticalSliderRotY->setMaximum(3600);
        verticalSliderRotY->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(verticalSliderRotY);

        horizontalSpacer = new QSpacerItem(8, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        verticalSliderZoom = new QSlider(groupBoxView);
        verticalSliderZoom->setObjectName(QStringLiteral("verticalSliderZoom"));
        verticalSliderZoom->setMaximum(1000);
        verticalSliderZoom->setValue(100);
        verticalSliderZoom->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(verticalSliderZoom);


        gridLayout->addWidget(groupBoxView, 0, 5, 2, 1);

        groupBoxLight = new QGroupBox(centralWidget);
        groupBoxLight->setObjectName(QStringLiteral("groupBoxLight"));
        sizePolicy.setHeightForWidth(groupBoxLight->sizePolicy().hasHeightForWidth());
        groupBoxLight->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(groupBoxLight);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalSliderLightX = new QSlider(groupBoxLight);
        verticalSliderLightX->setObjectName(QStringLiteral("verticalSliderLightX"));
        verticalSliderLightX->setMinimum(-100);
        verticalSliderLightX->setMaximum(100);
        verticalSliderLightX->setValue(10);
        verticalSliderLightX->setOrientation(Qt::Vertical);

        horizontalLayout_2->addWidget(verticalSliderLightX);

        verticalSliderLightY = new QSlider(groupBoxLight);
        verticalSliderLightY->setObjectName(QStringLiteral("verticalSliderLightY"));
        verticalSliderLightY->setMinimum(-100);
        verticalSliderLightY->setMaximum(100);
        verticalSliderLightY->setValue(30);
        verticalSliderLightY->setOrientation(Qt::Vertical);

        horizontalLayout_2->addWidget(verticalSliderLightY);

        verticalSliderLightZ = new QSlider(groupBoxLight);
        verticalSliderLightZ->setObjectName(QStringLiteral("verticalSliderLightZ"));
        verticalSliderLightZ->setMinimum(-100);
        verticalSliderLightZ->setMaximum(100);
        verticalSliderLightZ->setOrientation(Qt::Vertical);

        horizontalLayout_2->addWidget(verticalSliderLightZ);


        gridLayout->addWidget(groupBoxLight, 0, 6, 2, 1);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        groupBoxView->setTitle(QApplication::translate("MainWindow", "Widok", 0));
        groupBoxLight->setTitle(QApplication::translate("MainWindow", "\305\232wiat\305\202o", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
