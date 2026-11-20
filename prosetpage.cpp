#include "prosetpage.h"
#include "ui_prosetpage.h"
#include<QLineEdit>
#include<QDir>
#include<QFileDialog>
ProSetPage::ProSetPage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::ProSetPage)
{
    ui->setupUi(this);
    registerField("proPath",ui->lineEdit_2);
    registerField("proName*",ui->lineEdit);
    connect(ui->lineEdit,&QLineEdit::textEdited,this,&ProSetPage::completeChanged);
    connect(ui->lineEdit_2,&QLineEdit::textEdited,this,&ProSetPage::completeChanged);
    // ui->lineEdit->setStyleSheet("QLineEdit { color: black; }");
    // ui->lineEdit_2->setStyleSheet("QLineEdit { color: black; }");
    QString curPath=QDir::currentPath();
    ui->lineEdit_2->setText(curPath);
    ui->lineEdit_2->setCursorPosition(ui->lineEdit_2->text().size());
    ui->lineEdit_2->setClearButtonEnabled(true);
    ui->lineEdit->setClearButtonEnabled(true);
}

ProSetPage::~ProSetPage()
{
    delete ui;
}

void ProSetPage::GetProSettings(QString &name, QString &path)
{
    name=ui->lineEdit->text();
    path=ui->lineEdit_2->text();
}

bool ProSetPage::isComplete() const
{
    if(ui->lineEdit->text()==""||ui->lineEdit_2->text()=="")
        return false;
    QDir dir(ui->lineEdit_2->text());
        if(!dir.exists())
        {
        ui->tips->setText("project path is not exists");
            return false;
        }
        QString absFilePath=dir.absoluteFilePath(ui->lineEdit->text());
        QDir dist_dir(absFilePath);
        if(dist_dir.exists())
        {
            ui->tips->setText("项目已存在，请更换路径或名称。");
            return false;
        }
        ui->tips->setText("");
        return true;
}

void ProSetPage::on_pushButton_clicked()
{
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::Directory);
    file_dialog.setWindowTitle(tr("选择导入的文件夹"));
    auto path=QDir::currentPath();
    file_dialog.setDirectory(path);
    file_dialog.setViewMode(QFileDialog::Detail);
    QStringList filename;
    if(file_dialog.exec())
    {
        filename=file_dialog.selectedFiles();
    }
    if(filename.length()<=0)
    {
        return;
    }
    QString import_path=filename.at(0);
    ui->lineEdit_2->setText(import_path);
}

