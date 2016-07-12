#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <memory>
#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>
#include <QItemSelection>
#include "../common/common_defines.h"

class ProxySqlModel;
namespace Ui {
    class DBTEditor;
}
namespace dbi {
    class DBTInfo;
}

/*
 * The base class for editing database tables (DBT)
 */
class DBTEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTEditor();

    void selectInitial(const QVariant &idPrim);
    cmmn::T_id selectedId() const;

protected: // TODO: make private
    void setContentsUI();
    void setEditingUI();
    void setModel();
    void setWindowName();

    dbi::DBTInfo *m_DBTInfo;
    std::unique_ptr<Ui::DBTEditor> m_ui;
    std::unique_ptr<ProxySqlModel> m_proxyModel;
};

#endif // DBT_EDITOR_H
