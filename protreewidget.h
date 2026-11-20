#ifndef PROTREEWIDGET_H
#define PROTREEWIDGET_H

#include <QTreeWidget>
#include<QAction>
class ProTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    ProTreeWidget(QWidget*parent=nullptr);
    void AddProToTree(const QString&name,const QString&path);
    void AddProjectPath(const QString &name, const QString &projectPath);
    void RemoveProjectPath(const QString &projectPath);

signals:
    void projectSelected(const QString &path);
    void projectClosed(const QString &path);
    void importRequested(const QString &path);
    void slideshowRequested(const QString &path);

private:
    QSet<QString>_set_path;
    QTreeWidgetItem*_right_btn_item = nullptr;
    QAction*_action_import;
    QAction*_action_setstart;
    QAction*_action_closepro;
    QAction*_action_slideshow;
private slots:
    void SlotItemPressed(QTreeWidgetItem*item,int column);
    void SlotImport();
    void SlotSetStart();
    void SlotCloseProject();
    void SlotSlideshow();
    void SlotItemClicked(QTreeWidgetItem *item, int column);
};

#endif // PROTREEWIDGET_H
