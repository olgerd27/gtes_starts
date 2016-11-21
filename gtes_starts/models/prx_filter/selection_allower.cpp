#include <QDebug> // TODO: delete later
#include "selection_allower.h"
#include "../../dbt_editor/table_view_ds.h"

// SelectionAllower
SelectionAllower::SelectionAllower(QObject *parent)
    : QObject(parent)
{ }

bool SelectionAllower::isSelectionAllowed() const
{
    return true;
}

// SelectionAllower_IC
SelectionAllower_IC::SelectionAllower_IC(QLineEdit *le, QTableView *tblView, QObject *parent)
    : SelectionAllower(parent)
    , m_mousePressed(false)
    , m_allowSelection(true)
{
    /*
     * Principle of functionality:
     * - selection denieds when user change text in LineEdit.
     * - selection allows when changes text in LineEdit makes LineEdit empty (i.e. deletes a some last character) OR
     *   when user click on some cell in the TableView.
     */
    connect(le, &QLineEdit::textChanged, [this, tblView](const QString &text)
    {
        if ( !(m_allowSelection = text.isEmpty()) )
            tblView->clearSelection();
    });

    // define tableView signal for definition selection allow
    connect(qobject_cast<TableView_DS *>(tblView), &TableView_DS::sigMousePressedOverTable,
            [this]() { m_mousePressed = true; });

    connect(tblView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this](const QItemSelection &selected, const QItemSelection &)
    {
        if (m_mousePressed && !selected.indexes().isEmpty())
            m_allowSelection = true; // if mouse was pressed over table and selection changes
    });
}

bool SelectionAllower_IC::isSelectionAllowed() const
{
    return m_allowSelection;
}

void SelectionAllower_IC::slotSelectionEnded()
{
    m_mousePressed = false;
}
