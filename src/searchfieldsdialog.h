#ifndef SEARCHFIELDSDIALOG_H
#define SEARCHFIELDSDIALOG_H

#include <QDialog>
#include "searchfield.h"

namespace Ui {
    class SearchFieldsDialog;
}

class SearchFieldsDialog : public QDialog
{
    Q_OBJECT

public:
    SearchFieldsDialog(SearchField *searchfield, QWidget *parent = 0);
    ~SearchFieldsDialog();
    void loadSearchfields();

private slots:
    void on_toolEdit_clicked();
    void on_toolBDelete_clicked();

protected:
    Ui::SearchFieldsDialog *ui;
    SearchField *m_searchfield;
};

#endif // SEARCHFIELDSDIALOG_H
