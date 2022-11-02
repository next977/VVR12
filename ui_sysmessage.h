/********************************************************************************
** Form generated from reading UI file 'sysmessage.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYSMESSAGE_H
#define UI_SYSMESSAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_sysmessage
{
public:
    QLabel *System_label;
    QLabel *Action_No_label;
    QLabel *Danger_symbol;
    QLabel *Action_label;
    QLabel *Code_No_label;
    QLabel *Action_symbol;
    QFrame *line;
    QLabel *Code_label;

    void setupUi(QDialog *sysmessage)
    {
        if (sysmessage->objectName().isEmpty())
            sysmessage->setObjectName(QStringLiteral("sysmessage"));
        sysmessage->resize(534, 380);
        System_label = new QLabel(sysmessage);
        System_label->setObjectName(QStringLiteral("System_label"));
        System_label->setGeometry(QRect(100, 120, 421, 61));
        QFont font;
        font.setPointSize(30);
        System_label->setFont(font);
        Action_No_label = new QLabel(sysmessage);
        Action_No_label->setObjectName(QStringLiteral("Action_No_label"));
        Action_No_label->setGeometry(QRect(100, 290, 421, 51));
        QFont font1;
        font1.setPointSize(20);
        Action_No_label->setFont(font1);
        Danger_symbol = new QLabel(sysmessage);
        Danger_symbol->setObjectName(QStringLiteral("Danger_symbol"));
        Danger_symbol->setGeometry(QRect(20, 120, 61, 61));
        Action_label = new QLabel(sysmessage);
        Action_label->setObjectName(QStringLiteral("Action_label"));
        Action_label->setGeometry(QRect(100, 240, 101, 51));
        Action_label->setFont(font1);
        Code_No_label = new QLabel(sysmessage);
        Code_No_label->setObjectName(QStringLiteral("Code_No_label"));
        Code_No_label->setGeometry(QRect(120, 0, 101, 51));
        QFont font2;
        font2.setPointSize(25);
        Code_No_label->setFont(font2);
        Action_symbol = new QLabel(sysmessage);
        Action_symbol->setObjectName(QStringLiteral("Action_symbol"));
        Action_symbol->setGeometry(QRect(20, 260, 61, 61));
        line = new QFrame(sysmessage);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(0, 50, 541, 20));
        line->setFrameShadow(QFrame::Plain);
        line->setLineWidth(3);
        line->setFrameShape(QFrame::HLine);
        Code_label = new QLabel(sysmessage);
        Code_label->setObjectName(QStringLiteral("Code_label"));
        Code_label->setGeometry(QRect(20, 0, 101, 51));
        Code_label->setFont(font2);

        retranslateUi(sysmessage);

        QMetaObject::connectSlotsByName(sysmessage);
    } // setupUi

    void retranslateUi(QDialog *sysmessage)
    {
        sysmessage->setWindowTitle(QApplication::translate("sysmessage", "Dialog", Q_NULLPTR));
        System_label->setText(QApplication::translate("sysmessage", "None", Q_NULLPTR));
        Action_No_label->setText(QApplication::translate("sysmessage", "none", Q_NULLPTR));
        Danger_symbol->setText(QString());
        Action_label->setText(QApplication::translate("sysmessage", "Action : ", Q_NULLPTR));
        Code_No_label->setText(QApplication::translate("sysmessage", "None", Q_NULLPTR));
        Action_symbol->setText(QString());
        Code_label->setText(QApplication::translate("sysmessage", "Code : ", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class sysmessage: public Ui_sysmessage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSMESSAGE_H
