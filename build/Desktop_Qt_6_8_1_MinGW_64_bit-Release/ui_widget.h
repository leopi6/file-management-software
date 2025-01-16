/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *packageButton;
    QPushButton *unpackButton;
    QPushButton *compressButton;
    QPushButton *decompressButton;
    QPushButton *backupButton;
    QPushButton *restoreButton;
    QPushButton *startScheduledBackupButton;
    QPushButton *encryptButton;
    QPushButton *decryptButton;
    QLineEdit *passwordLineEdit;
    QPushButton *stopScheduledBackupButton;
    QPushButton *startMonitoringButton;
    QPushButton *stopMonitoringButton;
    QListWidget *fileListWidget;
    QListWidget *scheduledBackupListWidget;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(824, 368);
        packageButton = new QPushButton(Widget);
        packageButton->setObjectName("packageButton");
        packageButton->setGeometry(QRect(30, 220, 91, 31));
        unpackButton = new QPushButton(Widget);
        unpackButton->setObjectName("unpackButton");
        unpackButton->setGeometry(QRect(30, 260, 91, 31));
        compressButton = new QPushButton(Widget);
        compressButton->setObjectName("compressButton");
        compressButton->setGeometry(QRect(30, 70, 91, 31));
        decompressButton = new QPushButton(Widget);
        decompressButton->setObjectName("decompressButton");
        decompressButton->setGeometry(QRect(30, 120, 91, 31));
        backupButton = new QPushButton(Widget);
        backupButton->setObjectName("backupButton");
        backupButton->setGeometry(QRect(180, 220, 91, 31));
        restoreButton = new QPushButton(Widget);
        restoreButton->setObjectName("restoreButton");
        restoreButton->setGeometry(QRect(180, 260, 91, 31));
        startScheduledBackupButton = new QPushButton(Widget);
        startScheduledBackupButton->setObjectName("startScheduledBackupButton");
        startScheduledBackupButton->setGeometry(QRect(340, 220, 101, 31));
        encryptButton = new QPushButton(Widget);
        encryptButton->setObjectName("encryptButton");
        encryptButton->setGeometry(QRect(150, 70, 71, 41));
        decryptButton = new QPushButton(Widget);
        decryptButton->setObjectName("decryptButton");
        decryptButton->setGeometry(QRect(240, 70, 71, 41));
        passwordLineEdit = new QLineEdit(Widget);
        passwordLineEdit->setObjectName("passwordLineEdit");
        passwordLineEdit->setGeometry(QRect(150, 120, 161, 31));
        passwordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);
        stopScheduledBackupButton = new QPushButton(Widget);
        stopScheduledBackupButton->setObjectName("stopScheduledBackupButton");
        stopScheduledBackupButton->setGeometry(QRect(340, 260, 101, 31));
        startMonitoringButton = new QPushButton(Widget);
        startMonitoringButton->setObjectName("startMonitoringButton");
        startMonitoringButton->setGeometry(QRect(340, 70, 91, 31));
        stopMonitoringButton = new QPushButton(Widget);
        stopMonitoringButton->setObjectName("stopMonitoringButton");
        stopMonitoringButton->setGeometry(QRect(340, 120, 91, 31));
        fileListWidget = new QListWidget(Widget);
        fileListWidget->setObjectName("fileListWidget");
        fileListWidget->setGeometry(QRect(470, 70, 301, 81));
        fileListWidget->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
        scheduledBackupListWidget = new QListWidget(Widget);
        scheduledBackupListWidget->setObjectName("scheduledBackupListWidget");
        scheduledBackupListWidget->setGeometry(QRect(470, 220, 301, 71));
        scheduledBackupListWidget->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
        label = new QLabel(Widget);
        label->setObjectName("label");
        label->setGeometry(QRect(150, 150, 191, 21));
        label_2 = new QLabel(Widget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(470, 40, 131, 21));
        label_3 = new QLabel(Widget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(470, 190, 131, 21));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        packageButton->setText(QCoreApplication::translate("Widget", "\346\211\223\345\214\205", nullptr));
        unpackButton->setText(QCoreApplication::translate("Widget", "\350\247\243\345\214\205", nullptr));
        compressButton->setText(QCoreApplication::translate("Widget", "\345\216\213\347\274\251", nullptr));
        decompressButton->setText(QCoreApplication::translate("Widget", "\350\247\243\345\216\213", nullptr));
        backupButton->setText(QCoreApplication::translate("Widget", "\345\244\207\344\273\275", nullptr));
        restoreButton->setText(QCoreApplication::translate("Widget", "\350\277\230\345\216\237", nullptr));
        startScheduledBackupButton->setText(QCoreApplication::translate("Widget", "\345\256\232\346\227\266\345\244\207\344\273\275", nullptr));
        encryptButton->setText(QCoreApplication::translate("Widget", "\345\212\240\345\257\206", nullptr));
        decryptButton->setText(QCoreApplication::translate("Widget", "\350\247\243\345\257\206", nullptr));
        stopScheduledBackupButton->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242\345\256\232\346\227\266\345\244\207\344\273\275", nullptr));
        startMonitoringButton->setText(QCoreApplication::translate("Widget", "\347\233\221\350\247\206", nullptr));
        stopMonitoringButton->setText(QCoreApplication::translate("Widget", "\345\201\234\346\255\242\347\233\221\350\247\206", nullptr));
        label->setText(QCoreApplication::translate("Widget", "\350\257\267\345\234\250\344\270\212\346\226\271\350\276\223\345\205\245\345\212\240\345\257\206\346\210\226\350\247\243\345\257\206\345\257\206\347\240\201", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\347\233\221\350\247\206\345\210\227\350\241\250\342\206\223", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "\345\256\232\346\227\266\345\244\207\344\273\275\345\210\227\350\241\250\342\206\223", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
