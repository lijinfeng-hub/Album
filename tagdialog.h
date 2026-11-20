#ifndef TAGDIALOG_H
#define TAGDIALOG_H

#include <QDialog>

class QListWidget;
class QLineEdit;

class TagDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagDialog(const QStringList &allTags,
                       const QStringList &selectedTags,
                       QWidget *parent = nullptr);

    QStringList selectedTags() const;

private slots:
    void addTag();
    void renameTag();
    void removeTag();

private:
    void insertTag(const QString &tag, bool checked);

    QListWidget *_tagList = nullptr;
    QLineEdit *_tagEdit = nullptr;
};

#endif // TAGDIALOG_H
