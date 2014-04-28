#ifndef FILECHOOSERWIDGET_H
#define FILECHOOSERWIDGET_H

#include <qwidget.h>

class QToolButton;
class QLabel;
class QLineEdit;

class FileChooserWidget : public QWidget
{
    Q_OBJECT
public:
    FileChooserWidget(QWidget *parent = 0);

    void setLabelText(const QString &text);
    void setSettingName(const QString &name);

    QString getPath();
    QLineEdit *lineEdit();

    QLabel *label() const;

protected slots:
    void chooseFolder();

protected:
    QToolButton *m_button;
    QLabel *m_label;
    QLineEdit *m_edit;
    QString m_settingName;
};

#endif // FILECHOOSERWIDGET_H
