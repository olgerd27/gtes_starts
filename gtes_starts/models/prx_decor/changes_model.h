#ifndef CHANGES_MODEL_H
#define CHANGES_MODEL_H

#include <memory>
#include <QMap>

/*
 * RowsChangesHolder - storage of rows changes
 */
class RowsChangesHolder
{
public:
    typedef int T_rowNumber;

    enum ChangesTypes {
          chtype_insert
        , chtype_delete
        , chtype_alter
        , chtype_invalid
    };

    bool addChange(T_rowNumber row, ChangesTypes change);
    bool deleteChange(T_rowNumber row);
    void clearChanges();
    bool getChangeForRow(T_rowNumber row, ChangesTypes *pChangeType) const; // get the change for the specified row
    bool hasRowChange(T_rowNumber row, ChangesTypes controlChangeType) const; // checking - has a row change controlChangeType
    bool hasChanges() const;
private:
    void print() const; // TODO: for debugging, delete later

    QMap<T_rowNumber, ChangesTypes> m_rowsChanges;
};

/*
 * IRDefiner - definer indexes of row.
 * Use for definition row index, that need for switching to, after performing change model operations.
 */
class ProxyDecorModel;
class IRDefiner
{
public:
    enum DefinerType
    {
          dtype_insert          // row index definer type, used after calling "insert" operation
        , dtype_deleteExistent  // row index definer type, used after calling "delete" operation of existent in the DB row
        , dtype_deleteInserted  // row index definer type, used after calling "delete" operation of new inserted row
        , dtype_refreshExistent // row index definer type, used after calling "refresh" model operation, when current row is existent in DB row
        , dtype_refreshInserted // row index definer type, used after calling "refresh" model operation, when current row is new inserted row
        , dtype_save            // row index definer type, used after calling "save" model's data operation
    };

    IRDefiner(const ProxyDecorModel *model) : m_model(model) { }
    inline void setModel(const ProxyDecorModel *model) { m_model = model; }
    virtual bool define(int *pRow = 0) const = 0;

protected:
    const std::unique_ptr<RowsChangesHolder> & changesHolder() const;
    int modelRowsCount() const;

private:
    const ProxyDecorModel *m_model;
};

// Definer row index after performing insert operation
class IRD_Insert : public IRDefiner
{
public:
    IRD_Insert(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override;
};

// Definer row index after performing delete operation of just inserted row (not saved in the DB)
class IRD_DeleteExistent : public IRDefiner
{
private:
    typedef bool (IRD_DeleteExistent::*pFChangeRow)(int&) const;

    bool incr(int &row) const;
    bool decr(int &row) const;
    bool stop(int &) const;

public:
    IRD_DeleteExistent(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override;
};

// Definer row index after performing delete operation of existent row (saved in the DB)
class IRD_DeleteInserted : public IRDefiner
{
public:
    IRD_DeleteInserted(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override;
};

// Definer row index after performing refresh model operation - case if current row is a new inserted row and doesn't saved in the DB.
// ER - current row is Existent Row in the DB
class IRD_RefreshER : public IRDefiner
{
public:
    IRD_RefreshER(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override;
};

// Definer row index after performing refresh model operation - case if current row is existent in the DB row.
// IR - current row is Inserted Row
class IRD_RefreshIR : public IRDefiner
{
public:
    IRD_RefreshIR(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override;
};

// Definer row index after performing save model's data operation
class IRD_Save : public IRDefiner
{
public:
    IRD_Save(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override;
};

// The Factory method for getting particular row definer
IRDefiner *getIRDefiner(IRDefiner::DefinerType dtype, const ProxyDecorModel * const model);

#endif // CHANGES_MODEL_H
