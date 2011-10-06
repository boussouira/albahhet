#include "searchfieldsdialog.h"
#include "ui_searchfieldsdialog.h"
#include <QMessageBox>
#include <QInputDialog>

SearchFieldsDialog::SearchFieldsDialog(SearchField *searchfield, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchFieldsDialog),
    m_searchfield(searchfield)
{
    ui->setupUi(this);

    loadSearchfields();
}

SearchFieldsDialog::~SearchFieldsDialog()
{
    delete ui;
}

void SearchFieldsDialog::loadSearchfields()
{
    ui->listWidget->clear();

    foreach(SearchFieldInfo* field, m_searchfield->getFieldsNames()) {
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setText(field->name);
        item->setData(Qt::UserRole, field->fieldID);

        ui->listWidget->addItem(item);
    }
}

void SearchFieldsDialog::on_toolEdit_clicked()
{
    if(ui->listWidget->selectedItems().isEmpty()) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("لم تقم باختيار اي مجال بحث"));
        return;
    }

    QListWidgetItem *item = ui->listWidget->selectedItems().first();

    QString name = QInputDialog::getText(this,
                                         windowTitle(),
                                         tr("اسم مجال البحث:"),
                                         QLineEdit::Normal,
                                         item->text());
    if(!name.isEmpty()) {
        m_searchfield->setFieldName(name, item->data(Qt::UserRole).toInt());
        loadSearchfields();
    }
}

void SearchFieldsDialog::on_toolBDelete_clicked()
{
    if(ui->listWidget->selectedItems().isEmpty()) {
        QMessageBox::warning(this,
                             windowTitle(),
                             tr("لم تقم باختيار اي مجال بحث"));
        return;
    }

    QListWidgetItem *item = ui->listWidget->selectedItems().first();
    m_searchfield->removeield(item->data(Qt::UserRole).toInt());

    loadSearchfields();
}
