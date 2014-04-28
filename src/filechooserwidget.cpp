#include "filechooserwidget.h"

#include <qboxlayout.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtoolbutton.h>

FileChooserWidget::FileChooserWidget(QWidget *parent) :
    QWidget(parent)
{
    m_label = new QLabel(this);
    m_edit = new QLineEdit(this);
    m_button = new QToolButton(this);

    m_label->setText(QString::fromUtf8("مجلد وضع الملفات:"));
    m_button->setText("...");
    m_edit->setReadOnly(true);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_label);
    layout->addWidget(m_edit, 1);
    layout->addWidget(m_button);

    layout->setMargin(0);
    setLayout(layout);

    connect(m_button, SIGNAL(clicked()), SLOT(chooseFolder()));
}

void FileChooserWidget::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void FileChooserWidget::setSettingName(const QString &name)
{
    m_settingName = name;
}

QString FileChooserWidget::getPath()
{
    return m_edit->text();
}

QLineEdit *FileChooserWidget::lineEdit()
{
    return m_edit;
}

void FileChooserWidget::chooseFolder()
{
    QString lastPath = m_edit->text();
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("اختر مجلد"),
                                                        lastPath,
                                                        QFileDialog::ShowDirsOnly);
    if(dirPath.size()) {
        m_edit->setText(QDir::toNativeSeparators(dirPath));
    }
}
QLabel *FileChooserWidget::label() const
{
    return m_label;
}
