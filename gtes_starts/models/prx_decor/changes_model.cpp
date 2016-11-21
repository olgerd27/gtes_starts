#include <QDebug> // TODO: temp, delete later
#include "changes_model.h"
#include "proxy_decor_model.h"

/*
 * RowsChangesHolder
 */
bool RowsChangesHolder::addChange(RowsChangesHolder::T_rowNumber row, ChangesTypes changeNew)
{
    ChangesTypes changeContain = chtype_invalid; // change, that has passed row
    bool hasChange = getChangeForRow(row, &changeContain);
    if (!hasChange)
        m_rowsChanges.insert(row, changeNew);
    hasChange = !hasChange; // if initially row contains - this make bool value false, if doesn't contain - insert row and make bool value true

//    qDebug() << "Add change, row #" << row << ", changes:";
//    print();

    return hasChange;
}

bool RowsChangesHolder::deleteChange(RowsChangesHolder::T_rowNumber row)
{
    return m_rowsChanges.remove(row);
}

void RowsChangesHolder::clearChanges()
{
    m_rowsChanges.clear();
}

bool RowsChangesHolder::getChangeForRow(RowsChangesHolder::T_rowNumber row, ChangesTypes *pChangeType) const
{
    if (!hasChanges()) return false;
    ChangesTypes chtypeDefault = chtype_invalid;
    *pChangeType = m_rowsChanges.value(row, chtypeDefault);
    return *pChangeType != chtypeDefault;
}

bool RowsChangesHolder::hasRowChange(RowsChangesHolder::T_rowNumber row, ChangesTypes controlChangeType) const
{
    ChangesTypes changeType = chtype_invalid;
    return getChangeForRow(row, &changeType) && changeType == controlChangeType;
}

bool RowsChangesHolder::hasChanges() const
{
    return !m_rowsChanges.isEmpty();
}

void RowsChangesHolder::print() const
{
    QString name;
    for (auto it = m_rowsChanges.cbegin(); it != m_rowsChanges.cend(); ++it) {
        if (it.value() == chtype_insert) name = "INSERT";
        else if (it.value() == chtype_delete) name = "DELETE";
        else if (it.value() == chtype_invalid) name = "INVALID";
        else name = "UNKNOWN";
        qDebug() << "  " << it.key() << ":" << name;
    }
}

/*
 * The IRDefiner class hierarchy
 */
// IRDefiner - base class
const std::unique_ptr<RowsChangesHolder> & IRDefiner::changesHolder() const
{
    return m_model->m_changedRows;
}

int IRDefiner::modelRowsCount() const
{
    return m_model->rowCount();
}

// IRD_Insert
bool IRD_Insert::define(int *) const
{
    return true; // return true - this mean defined row is a new inserted row
}

// IRD_DeleteExistent
bool IRD_DeleteExistent::incr(int &row) const
{
    ++row;
    return true;
}

bool IRD_DeleteExistent::decr(int &row) const
{
    --row;
    return true;
}

bool IRD_DeleteExistent::stop(int &) const
{
    return false;
}

bool IRD_DeleteExistent::define(int *pRow) const
{
    int rowDef = *pRow; // copy for row definition
    int rowsCounter = 0;
    pFChangeRow fChangeRow = &IRD_DeleteExistent::decr; // set function, that decrement row index
    do {
        if (rowDef == 0) {
            fChangeRow = &IRD_DeleteExistent::incr; // set function, that increment row index
            rowDef = *pRow; // this allow don't repeat going by the same rows after change "change row" function to incr()
        }
        else if (rowDef == modelRowsCount())
            fChangeRow = &IRD_DeleteExistent::stop; // set function, that stop of row index changing
        if (++rowsCounter == modelRowsCount()) return false; // no one row can be defined -> exit from function with false
        if ( !(this->*fChangeRow)(rowDef) ) break; // make change of row index
    } while( changesHolder()->hasRowChange(rowDef, RowsChangesHolder::chtype_delete) );
    *pRow = rowDef;
    return true;
}

// IRD_DeleteInserted
bool IRD_DeleteInserted::define(int *pRow) const
{
    do {
        if (*pRow == 0) return false;
        --(*pRow);
    } while ( changesHolder()->hasRowChange(*pRow, RowsChangesHolder::chtype_delete) );
    return true;
}

// IRD_RefreshER
bool IRD_RefreshER::define(int *) const
{
    return true; // don't change current row
}

// IRD_RefreshIR
bool IRD_RefreshIR::define(int *pRow) const
{
//        *pRow = 0; // change current row to the first row
    *pRow = modelRowsCount() - 1; // change current row to the last existent in DB row
    return true;
}

// IRD_Save
bool IRD_Save::define(int *) const
{
    return true; // return true - this mean defined row is current row -> after saving, the current row won't change
}

// getIRDefiner - the factory Method pattern - creation a particular row definer
IRDefiner *getIRDefiner(IRDefiner::DefinerType dtype, const ProxyDecorModel * const model)
{
    IRDefiner *ird = 0;
    switch (dtype) {
    case IRDefiner::dtype_insert:
        ird = new IRD_Insert(model);
        break;
    case IRDefiner::dtype_deleteExistent:
        ird = new IRD_DeleteExistent(model);
        break;
    case IRDefiner::dtype_deleteInserted:
        ird = new IRD_DeleteInserted(model);
        break;
    case IRDefiner::dtype_refreshExistent:
        ird = new IRD_RefreshER(model);
        break;
    case IRDefiner::dtype_refreshInserted:
        ird = new IRD_RefreshIR(model);
        break;
    case IRDefiner::dtype_save:
        ird = new IRD_Save(model);
        break;
    default:
        ASSERT_DBG( 0, cmmn::MessageException::type_fatal, QObject::tr("Error index of row"),
                    QObject::tr("Cannot define index of row. Unknow model change operation."),
                    QString("getIRDefiner") );
        break;
    }
    return ird;
}
