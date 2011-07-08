#ifndef QURANINDEXERWIDGET_H
#define QURANINDEXERWIDGET_H

#include "abstractindexingwidget.h"

namespace Ui {
    class QuranIndexerWidget;
}
class IndexInfo;

class QuranIndexerWidget : public AbstractIndexingWidget
{
    Q_OBJECT

public:
    QuranIndexerWidget(QWidget *parent = 0);
    ~QuranIndexerWidget();
    QString indexTypeName();

protected:
    void readPaths();
    void checkIndex();
    void showBooks();
    void startIndexing();
    void stopIndexing();
    void saveIndexInfo(int indexingTime=0, int opimizingTime=0);

public slots:
    void nextStep();
    void addBook(const QString &);
    void doneIndexing();
    void indexingError();

private slots:
    void on_buttonSelectIndexPath_clicked();
    void on_buttonSelectShamela_clicked();
    void on_label_2_linkActivated(QString);

private:
    IndexInfo *m_indexInfo;
    Ui::QuranIndexerWidget *ui;
};

#endif // QURANINDEXERWIDGET_H
