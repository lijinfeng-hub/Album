#include "protree.h"
#include "ui_protree.h"

ProTree::ProTree(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProTree)
{
    ui->setupUi(this);
    this->setMinimumWidth(378);
    this->setMaximumWidth(378);
    connect(ui->treeWidget, &ProTreeWidget::projectSelected,
            this, &ProTree::projectSelected);
    connect(ui->treeWidget, &ProTreeWidget::projectClosed,
            this, &ProTree::projectClosed);
    connect(ui->treeWidget, &ProTreeWidget::importRequested,
            this, &ProTree::importRequested);
    connect(ui->treeWidget, &ProTreeWidget::slideshowRequested,
            this, &ProTree::slideshowRequested);
}

ProTree::~ProTree()
{
    delete ui;
}

void ProTree::AddProToTree(const QString &name, const QString &path)
{
    ui->treeWidget->AddProToTree(name,path);
}

void ProTree::AddProjectPath(const QString &name, const QString &projectPath)
{
    ui->treeWidget->AddProjectPath(name, projectPath);
}

void ProTree::RemoveProjectPath(const QString &projectPath)
{
    for (int index = 0; index < ui->treeWidget->topLevelItemCount(); ++index) {
        QTreeWidgetItem *item = ui->treeWidget->topLevelItem(index);
        if (item->data(0, Qt::UserRole).toString() == projectPath) {
            delete ui->treeWidget->takeTopLevelItem(index);
            return;
        }
    }
}
