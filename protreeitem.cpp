#include "protreeitem.h"




ProTreeItem::ProTreeItem(QTreeWidget *view, const QString &name, const QString &path, int type)
    :QTreeWidgetItem(view,type),_root(this),_path(path),_name(name),_pre_item(nullptr),_next_item(nullptr)
{

}

ProTreeItem::ProTreeItem(QTreeWidgetItem *parent, const QString &name,
                         const QString &path, QTreeWidgetItem *root, int type)
    :QTreeWidgetItem(parent,type),_root(root),_name(name),_path(path),_pre_item(nullptr),_next_item(nullptr)
{

}

const QString &ProTreeItem::getPath()
{
    return _path;
}

QTreeWidgetItem *ProTreeItem::getRoot()
{
    return _root;
}

void ProTreeItem::setPreItem(QTreeWidgetItem *item)
{
    _pre_item = item;
}
