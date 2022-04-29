#ifndef WIDGET_H
#define WIDGET_H

#include "des.h"
#include "aes.h"

#include <QWidget>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QString path;
    QString savepath;

private slots:
    void on_toolButton_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_toolButton_2_clicked();


private:
    Ui::Widget *ui;

    DES* des = new DES();

};
#endif // WIDGET_H
