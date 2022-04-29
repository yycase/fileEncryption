#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

}

Widget::~Widget()
{
    delete ui;
}

//选择文件按钮
void Widget::on_toolButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("选择文件"),
                                                "C:/Users/kaji/Desktop/111",
                                                tr("Files (*.txt);;"
                                                   "videos (*.mp4);;"
                                                   "Images (*.jpg *.png)"));
    ui->lineEdit->setText(path);
}
//保存文件按钮
void Widget::on_toolButton_2_clicked()
{
    QString savepath = QFileDialog::getExistingDirectory(this,"保存文件","C:/Users/kaji/Desktop");

    ui->lineEdit_2->setText(savepath);

}

void Widget::on_pushButton_clicked() //加密按钮
{   
    string filename = ui->lineEdit->text().toStdString();
    string savefile = ui->lineEdit_2->text().toStdString();

    if(filename.length()==0)
    {
        QMessageBox::information(NULL, tr("Notice"), tr("未选择文件."));
        return;
    }
    if(savefile.length()==0)
    {
        QMessageBox::information(NULL, tr("Notice"), tr("未选择保存路径."));
        return;
    }


    if(ui->comboBox->currentText()=="DES")
    {
        string saveFilePath = des->encryptFile(filename,savefile);

        QMessageBox::information(NULL, tr("Notice"), tr("DES加密成功，文件已保存到: ") + QString::fromStdString(saveFilePath));
    }
    else
    {
        string saveFilePath = encryptFile(filename,savefile);

        QMessageBox::information(NULL, tr("Notice"), tr("AES加密成功，文件已保存到: ") + QString::fromStdString(saveFilePath));
    }
}


void Widget::on_pushButton_2_clicked()  // 解密按钮
{
    string filename = ui->lineEdit->text().toStdString();
    string savefile = ui->lineEdit_2->text().toStdString();


    if(filename.length()==0)
    {
        QMessageBox::information(NULL, tr("Notice"), tr("未选择文件."));
        return;
    }
    if(savefile.length()==0)
    {
        QMessageBox::information(NULL, tr("Notice"), tr("未选择保存路径."));
        return;
    }


    if(ui->comboBox->currentText()=="DES"){

        string saveFilePath = des->decryptFile(filename,savefile);

        QMessageBox::information(NULL, tr("Notice"), tr("DES解密成功，文件已保存到: ") + QString::fromStdString(saveFilePath));
    }
    else
    {
        string saveFilePath = decryptFile(filename,savefile);

        QMessageBox::information(NULL, tr("Notice"), tr("AES解密成功，文件已保存到: ") + QString::fromStdString(saveFilePath));

    };

}







