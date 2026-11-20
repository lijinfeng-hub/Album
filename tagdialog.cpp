#include "tagdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

TagDialog::TagDialog(const QStringList &allTags,
                     const QStringList &selectedTags,
                     QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("编辑标签"));
    resize(360, 420);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("选择标签，也可以新增、重命名或删除标签："), this));

    _tagList = new QListWidget(this);
    for (const QString &tag : allTags) {
        insertTag(tag, selectedTags.contains(tag));
    }
    layout->addWidget(_tagList, 1);

    auto *editLayout = new QHBoxLayout;
    _tagEdit = new QLineEdit(this);
    _tagEdit->setPlaceholderText(tr("输入标签名称"));
    auto *addButton = new QPushButton(tr("新增"), this);
    auto *renameButton = new QPushButton(tr("重命名"), this);
    auto *removeButton = new QPushButton(tr("删除"), this);
    editLayout->addWidget(_tagEdit, 1);
    editLayout->addWidget(addButton);
    editLayout->addWidget(renameButton);
    editLayout->addWidget(removeButton);
    layout->addLayout(editLayout);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(addButton, &QPushButton::clicked, this, &TagDialog::addTag);
    connect(renameButton, &QPushButton::clicked, this, &TagDialog::renameTag);
    connect(removeButton, &QPushButton::clicked, this, &TagDialog::removeTag);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QStringList TagDialog::selectedTags() const
{
    QStringList tags;
    for (int index = 0; index < _tagList->count(); ++index) {
        QListWidgetItem *item = _tagList->item(index);
        if (item->checkState() == Qt::Checked) {
            tags.append(item->text());
        }
    }
    return tags;
}

void TagDialog::addTag()
{
    const QString tag = _tagEdit->text().trimmed();
    if (tag.isEmpty()) {
        return;
    }
    for (int index = 0; index < _tagList->count(); ++index) {
        if (_tagList->item(index)->text().compare(tag, Qt::CaseInsensitive) == 0) {
            _tagList->item(index)->setCheckState(Qt::Checked);
            _tagEdit->clear();
            return;
        }
    }
    insertTag(tag, true);
    _tagEdit->clear();
}

void TagDialog::renameTag()
{
    QListWidgetItem *item = _tagList->currentItem();
    const QString tag = _tagEdit->text().trimmed();
    if (!item || tag.isEmpty()) {
        return;
    }
    for (int index = 0; index < _tagList->count(); ++index) {
        if (_tagList->item(index) != item
            && _tagList->item(index)->text().compare(tag, Qt::CaseInsensitive) == 0) {
            return;
        }
    }
    item->setText(tag);
    _tagEdit->clear();
}

void TagDialog::removeTag()
{
    delete _tagList->takeItem(_tagList->currentRow());
}

void TagDialog::insertTag(const QString &tag, bool checked)
{
    auto *item = new QListWidgetItem(tag, _tagList);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}
