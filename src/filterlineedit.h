#ifndef FILTERLINEEDIT_H
#define FILTERLINEEDIT_H

#include "fancylineedit.h"

class FilterLineEdit : public FancyLineEdit
{
    Q_OBJECT
public:
    explicit FilterLineEdit(QWidget *parent = 0);

    void setFilterMenu(QMenu *menu);

signals:
    void filterChanged(const QString &);
    void delayFilterChanged(const QString &);
    void delayFilterChanged();
    void filterClear();

private slots:
    void slotTextChanged();
    void slotDelayTextChanged();

private:
    QString m_lastFilterText;
    QTimer *m_filterTimer;
};

#endif // FILTERLINEEDIT_H
