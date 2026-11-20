#ifndef PROTREE_H
#define PROTREE_H

#include <QDialog>

namespace Ui {
class ProTree;
}

class ProTree : public QDialog
{
    Q_OBJECT

public:
    explicit ProTree(QWidget *parent = nullptr);
    ~ProTree();

    void AddProjectPath(const QString &name, const QString &projectPath);
    void RemoveProjectPath(const QString &projectPath);

signals:
    void projectSelected(const QString &path);
    void projectClosed(const QString &path);
    void importRequested(const QString &path);
    void slideshowRequested(const QString &path);

private:
    Ui::ProTree *ui;
public slots:
    void AddProToTree(const QString&name,const QString&path);
};

#endif // PROTREE_H
