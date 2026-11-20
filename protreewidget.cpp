#include "protreewidget.h"
#include<QDir>
#include"protreeitem.h"
#include"const.h"
#include<QGuiApplication>
#include<QMenu>
#include<QFileDialog>
#include<QMessageBox>

ProTreeWidget::ProTreeWidget(QWidget *parent):QTreeWidget(parent)
{
    connect(this,&ProTreeWidget::itemPressed,this,&ProTreeWidget::SlotItemPressed);
    connect(this,&QTreeWidget::itemClicked,this,&ProTreeWidget::SlotItemClicked);
    _action_import=new QAction(QIcon(":/Icon/import.png"),tr("导入文件"),this);
    _action_setstart=new QAction(QIcon(":/Icon/项目设置—项目管理.png"),tr("设置活动项目"),this);
    _action_closepro=new QAction(QIcon(":/Icon/关闭 (1).png"),tr("关闭项目"),this);
    _action_slideshow=new QAction(QIcon(":/Icon/slide.png"),tr("轮播图播放"),this);
    connect(_action_import,&QAction::triggered,this,&ProTreeWidget::SlotImport);
    connect(_action_setstart,&QAction::triggered,this,&ProTreeWidget::SlotSetStart);
    connect(_action_closepro,&QAction::triggered,this,&ProTreeWidget::SlotCloseProject);
    connect(_action_slideshow,&QAction::triggered,this,&ProTreeWidget::SlotSlideshow);
}

void ProTreeWidget::AddProToTree(const QString &name, const QString &path)
{
    QDir dir(path);
    QString file_path=dir.absoluteFilePath(name);
    AddProjectPath(name, file_path);
}

void ProTreeWidget::AddProjectPath(const QString &name, const QString &projectPath)
{
    const QString file_path = QDir(projectPath).absolutePath();
    if(_set_path.find(file_path)!=_set_path.end())
    {
        return ;
    }
    if (!QDir(file_path).exists()) return;
    _set_path.insert(file_path);
    auto*item=new ProTreeItem(this,name,file_path,TreeItemPro);
    item->setData(0,Qt::DisplayRole,name);
    item->setData(0,Qt::DecorationRole,QIcon(":/Icon/dir.png"));
    item->setData(0,Qt::ToolTipRole,file_path);
    item->setData(0,Qt::UserRole,file_path);
    this->addTopLevelItem(item);
    setCurrentItem(item);
}

void ProTreeWidget::RemoveProjectPath(const QString &projectPath)
{
    const QString filePath = QDir(projectPath).absolutePath();
    _set_path.remove(filePath);

    for (int index = 0; index < topLevelItemCount(); ++index) {
        QTreeWidgetItem *item = topLevelItem(index);
        if (item->data(0, Qt::UserRole).toString() == filePath) {
            delete takeTopLevelItem(index);
            return;
        }
    }
}

void ProTreeWidget::SlotItemPressed(QTreeWidgetItem *presseditem, int column)
{
    if(QGuiApplication::mouseButtons()==Qt::RightButton)
    {
         QMenu menu(this);
        int itemtype=presseditem->type();
        if(itemtype==TreeItemPro)
        {
            _right_btn_item=presseditem;
            menu.addAction(_action_import);
            menu.addAction(_action_setstart);
            menu.addAction(_action_closepro);
            menu.addAction(_action_slideshow);
            menu.exec(QCursor::pos());
        }
    }
}

void ProTreeWidget::SlotImport()
{
    if(!_right_btn_item)
    {
        return;
    }
    emit importRequested(_right_btn_item->data(0, Qt::UserRole).toString());
}

void ProTreeWidget::SlotSetStart()
{
    if (_right_btn_item) {
        setCurrentItem(_right_btn_item);
        emit projectSelected(_right_btn_item->data(0, Qt::UserRole).toString());
    }
}

void ProTreeWidget::SlotCloseProject()
{
    if (!_right_btn_item) return;
    const QString path = _right_btn_item->data(0, Qt::UserRole).toString();
    _set_path.remove(path);
    delete takeTopLevelItem(indexOfTopLevelItem(_right_btn_item));
    _right_btn_item = nullptr;
    emit projectClosed(path);
}

void ProTreeWidget::SlotSlideshow()
{
    if (_right_btn_item) {
        emit slideshowRequested(_right_btn_item->data(0, Qt::UserRole).toString());
    }
}

void ProTreeWidget::SlotItemClicked(QTreeWidgetItem *item, int)
{
    if (item && item->type() == TreeItemPro) {
        emit projectSelected(item->data(0, Qt::UserRole).toString());
    }
}
