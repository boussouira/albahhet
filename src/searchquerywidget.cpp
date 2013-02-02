#include "searchquerywidget.h"
#include "ui_searchquerywidget.h"
#include "common.h"
#include "cl_common.h"
#include "arabicanalyzer.h"

#include <qmessagebox.h>
#include <qsettings.h>
#include <qstringlistmodel.h>
#include <qcompleter.h>
#include <qmenu.h>

Query *parse(QueryParser *queryPareser, const QString &text, bool andOperator)
{
    if(text.isEmpty())
        return 0;

    queryPareser->setDefaultOperator(andOperator ? QueryParser::AND_OPERATOR
                                                 : QueryParser::OR_OPERATOR);

    Query *query = 0;
    wchar_t *queryText = QStringToTChar(text);
    try {
        query = queryPareser->parse(queryText);
    } catch(CLuceneError &) {
        free(queryText);

        queryText = QueryParser::escape(QStringToTChar(text));
        query = queryPareser->parse(queryText);

        free(queryText);
    }

    return query;
}

QString clearSpecialChars(const QString &text)
{
    wchar_t *lineText = QStringToTChar(text);
    wchar_t *cleanText = QueryParser::escape(lineText);

    QString clearText = QString::fromWCharArray(cleanText);

    free(lineText);
    free(cleanText);

    return clearText;
}

SearchQueryWidget::SearchQueryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchQueryWidget)
{
    ui->setupUi(this);

    m_advancedSearch = false;
    m_completer = 0;

    setupCleanMenu();
    loadSearchQuery();
    updateSearchWidget();

    connect(ui->toolAdvancedSearch,
            SIGNAL(clicked()),
            SLOT(toggoleAdvancedSearchWidget()));

    connect(ui->labelAdvancedSearch,
            SIGNAL(linkActivated(QString)),
            SLOT(toggoleAdvancedSearchWidget()));

    connect(ui->lineDefaultQuery, SIGNAL(returnPressed()), SIGNAL(search()));
    connect(ui->lineAllWordsQuery, SIGNAL(returnPressed()), SIGNAL(search()));
    connect(ui->lineAnyWordQuery, SIGNAL(returnPressed()), SIGNAL(search()));
    connect(ui->lineExactQuery, SIGNAL(returnPressed()), SIGNAL(search()));
    connect(ui->lineWithoutQuery, SIGNAL(returnPressed()), SIGNAL(search()));
}

SearchQueryWidget::~SearchQueryWidget()
{
    delete ui;
}

Query *SearchQueryWidget::searchQuery(QueryParser *queryPareser)
{
    try {

        if(m_advancedSearch) {
            return advancedQuery(queryPareser);
        } else {
            return defaultQuery(queryPareser);
        }

    } catch(CLuceneError &e) {
        if(e.number() == CL_ERR_Parse)
            QMessageBox::warning(this,
                                 tr("خطأ في استعلام البحث"),
                                 tr("هنالك خطأ في احدى حقول البحث"
                                    "\n"
                                    "تأكد من حذف الأقواس و المعقوفات وغيرها،"
                                    " ويمكنك فعل ذلك من خلال زر التنظيف الموجود يسار حقل البحث، بعد الضغط على هذا الزر اعد البحث"
                                    "\n"
                                    "او تأكد من أنك تستخدمها بشكل صحيح"));
        else
            QMessageBox::warning(0,
                                 "CLucene Query error",
                                 tr("code: %1\nError: %2").arg(e.number()).arg(e.what()));

    }
    catch(...) {
        QMessageBox::warning(0,
                             "CLucene Query error",
                             tr("Unknow error"));
    }

    return 0;
}

void SearchQueryWidget::setSearchQuery(const QString &text)
{
    if(m_advancedSearch)
        ui->lineAllWordsQuery->setText(text);
    else
        ui->lineDefaultQuery->setText(text);
}

void SearchQueryWidget::loadSettings()
{
    QSettings settings;

    settings.beginGroup("Search");

    m_advancedSearch = settings.value("advancedSearch", false).toBool();

    ui->checkDefaultQuery->setChecked(settings.value("checkDefaultQuery", false).toBool());
    ui->checkWithoutQuery->setChecked(settings.value("checkWithoutQuery", false).toBool());

    updateSearchWidget();
}

void SearchQueryWidget::saveSettings()
{
    QSettings settings;

    settings.beginGroup("Search");

    settings.setValue("advancedSearch", m_advancedSearch);
    settings.setValue("checkDefaultQuery", ui->checkDefaultQuery->isChecked());
    settings.setValue("checkWithoutQuery", ui->checkWithoutQuery->isChecked());
}

void SearchQueryWidget::loadSearchQuery()
{
}

void SearchQueryWidget::saveSearchQuery()
{
}

Query *SearchQueryWidget::defaultQuery(QueryParser *queryPareser)
{
    QString qureyStr = ui->lineDefaultQuery->text().trimmed();

    if(qureyStr.isEmpty()){
            QMessageBox::warning(this,
                                 tr("البحث"),
                                 tr("لم تدخل أي كلمة ليتم البحث عنها"));
            return 0;
    }

    return parse(queryPareser,
                 qureyStr,
                 ui->checkDefaultQuery->isChecked());
}

Query *SearchQueryWidget::advancedQuery(QueryParser *queryPareser)
{
    if(ui->lineAllWordsQuery->text().trimmed().isEmpty()
            && ui->lineAnyWordQuery->text().trimmed().isEmpty()
            && ui->lineExactQuery->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("البحث"),
                             tr("لم تدخل أي كلمة ليتم البحث عنها"));
        return 0;
    }

    // All words query
    Query *allWordsQuery = parse(queryPareser,
                                 ui->lineAllWordsQuery->text().trimmed(),
                                 true);

    Query *anyWordQuery = parse(queryPareser,
                                ui->lineAnyWordQuery->text().trimmed(),
                                false);

    Query *exactQuery = 0;
    if(ui->lineExactQuery->text().trimmed().size()) {
        // The query parser do a lot of stuff for phrase query
        QString exactQueryText = QString("\"%1\"").arg(clearSpecialChars(ui->lineExactQuery->text().trimmed()));

        if(ui->spinExactQuerySlop->value())
            exactQueryText.append(QString("~%1").arg(ui->spinExactQuerySlop->value()));

        exactQuery = parse(queryPareser, exactQueryText, true);
    }

    Query *withoutQuery = parse(queryPareser,
                                ui->lineWithoutQuery->text().trimmed(),
                                ui->checkWithoutQuery->isChecked());

    if(!allWordsQuery && !anyWordQuery && !exactQuery)
        return 0;

    BooleanQuery *query = new BooleanQuery;

    if(allWordsQuery)
        query->add(allWordsQuery, BooleanClause::MUST);

    if(anyWordQuery)
        query->add(anyWordQuery, BooleanClause::MUST);

    if(exactQuery)
        query->add(exactQuery, BooleanClause::MUST);

    if(withoutQuery)
        query->add(withoutQuery, BooleanClause::MUST_NOT);

    return query;
}

void SearchQueryWidget::toggoleAdvancedSearchWidget()
{
    m_advancedSearch = !m_advancedSearch;
    updateSearchWidget();
}

void SearchQueryWidget::setupCleanMenu()
{
    QList<FilterLineEdit*> lines;
    lines << ui->lineDefaultQuery
          << ui->lineAllWordsQuery
          << ui->lineAnyWordQuery
          << ui->lineExactQuery
          << ui->lineWithoutQuery;

    foreach(FilterLineEdit *line, lines) {
        QMenu *menu = new QMenu(line);
        QAction *clearSpecialCharAct = new QAction(tr("ابطال مفعول الاقواس وغيرها"), line);
        QAction *removeTashekilAct = new QAction(tr("حذف التشكيل"), line);

        if(line != ui->lineExactQuery) {
            menu->addAction(clearSpecialCharAct);
            menu->addSeparator();
        }

        menu->addAction(removeTashekilAct);

        connect(clearSpecialCharAct, SIGNAL(triggered()), SLOT(clearSpecialChar()));
        connect(removeTashekilAct, SIGNAL(triggered()), SLOT(removeTashekil()));

        line->setFilterMenu(menu);
    }
}

void SearchQueryWidget::clearSpecialChar()
{
    FilterLineEdit *edit = qobject_cast<FilterLineEdit*>(sender()->parent());

    if(edit) {
        edit->setText(clearSpecialChars(edit->text()));
    }
}

void SearchQueryWidget::matchSearch()
{
    FilterLineEdit *edit = qobject_cast<FilterLineEdit*>(sender()->parent());

    if(edit) {
        QString text = edit->text().trimmed();
        if(text.isEmpty())
            return;

        QChar del('"');
        if(text.startsWith(del) && text.endsWith(del)) {
            text = text.remove(0, 1).remove(text.size()-2, 1);
        } else {
            if(!text.startsWith(del))
                text.prepend(del);

            if(!text.endsWith(del))
                text.append(del);
        }

        edit->setText(text);
    }
}

void SearchQueryWidget::removeTashekil()
{
    FilterLineEdit *edit = qobject_cast<FilterLineEdit*>(sender()->parent());

    if(edit) {
        edit->setText(edit->text().remove(QRegExp("[\\x064B-\\x0653]")).trimmed());
    }
}

void SearchQueryWidget::updateSearchWidget()
{
    ui->widgetAdvancedSearch->setVisible(m_advancedSearch);
    ui->toolAdvancedSearch->setText((m_advancedSearch ? "-" : "+"));
    ui->lineDefaultQuery->setVisible(!m_advancedSearch);
    ui->checkDefaultQuery->setVisible(!m_advancedSearch);
    ui->labelDefaultQuery->setVisible(!m_advancedSearch);
}
