/********************************************************************************
** Form generated from reading UI file 'flashingorder.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FLASHINGORDER_H
#define UI_FLASHINGORDER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FlashingOrderClass
{
public:

    void setupUi(QWidget *FlashingOrderClass)
    {
        if (FlashingOrderClass->objectName().isEmpty())
            FlashingOrderClass->setObjectName(QStringLiteral("FlashingOrderClass"));
        FlashingOrderClass->resize(600, 400);

        retranslateUi(FlashingOrderClass);

        QMetaObject::connectSlotsByName(FlashingOrderClass);
    } // setupUi

    void retranslateUi(QWidget *FlashingOrderClass)
    {
        FlashingOrderClass->setWindowTitle(QApplication::translate("FlashingOrderClass", "FlashingOrder", 0));
    } // retranslateUi

};

namespace Ui {
    class FlashingOrderClass: public Ui_FlashingOrderClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FLASHINGORDER_H
