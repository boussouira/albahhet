#include "filterlineedit.h"
#include <qtimer.h>
#include <qicon.h>
/*!
    \class Utils::FilterLineEdit

    \brief A fancy line edit customized for filtering purposes with a clear button.
*/

FilterLineEdit::FilterLineEdit(QWidget *parent) :
   FancyLineEdit(parent),
   m_lastFilterText(text()),
   m_filterTimer(0)
{
    // KDE has custom icons for this. Notice that icon namings are counter intuitive.
    // If these icons are not available we use the freedesktop standard name before
    // falling back to a bundled resource.
    QIcon clearIcon = QIcon::fromTheme(layoutDirection() == Qt::LeftToRight ?
                     QLatin1String("edit-clear-locationbar-rtl") :
                     QLatin1String("edit-clear-locationbar-ltr"),
                     QIcon::fromTheme(QLatin1String("edit-clear"), QIcon(QLatin1String(":/data/images/clear.png"))));

    QIcon menuIcon(":/data/images/magnifier.png");

    //setPlaceholderText(tr("بحث"));

    setButtonToolTip(Right, tr("حذف النص"));
    setButtonPixmap(Right, clearIcon.pixmap(16));
    setButtonVisible(Right, true);
    setAutoHideButton(Right, true);

    setButtonPixmap(Left, menuIcon.pixmap(16));

    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(300);

    connect(m_filterTimer, SIGNAL(timeout()), SLOT(slotDelayTextChanged()));
    connect(this, SIGNAL(rightButtonClicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged()));
    connect(this, SIGNAL(returnPressed()), SIGNAL(delayFilterChanged()));
}

void FilterLineEdit::setFilterMenu(QMenu *menu)
{
    setButtonMenu(Left, menu);
    setButtonVisible(Left, true);
}

void FilterLineEdit::slotTextChanged()
{
    const QString newlyTypedText = text();

    if(newlyTypedText.isEmpty())
        emit filterClear();

    if (newlyTypedText != m_lastFilterText) {
        m_lastFilterText = newlyTypedText;
        emit filterChanged(m_lastFilterText);

        m_filterTimer->setInterval((m_lastFilterText.size() > 9 ? 300 : 900));
        m_filterTimer->start();
    }
}

void FilterLineEdit::slotDelayTextChanged()
{
    emit delayFilterChanged();
    emit delayFilterChanged(text());
}
