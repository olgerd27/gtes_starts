#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <memory>
#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>
#include <QItemSelection>
#include <QDataWidgetMapper>
#include <QLabel> // TODO: delete is not need
#include "../common/common_defines.h"

class ProxyChoiceDecorModel;
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
    explicit DBTEditor(const dbi::DBTInfo *dbTable, QWidget *parent = 0);
    ~DBTEditor();

    void selectInitial(const QVariant &idPrim);
    cmmn::T_id selectedId() const;

private slots:
    void slotFocusLost_DataSet(const QString &data);

private:
    void setWindowName();
    void setModel();
    void setMapper();
    void setSelectUI();
    void setEditingUI();
    void setControl();
    void setDataNavigation();

    const dbi::DBTInfo *m_DBTInfo;
//    std::unique_ptr<Ui::DBTEditor> m_ui; // use in the release mode
//    std::unique_ptr<ProxyChoiceDecorModel> m_proxyModel; // use in the release mode
    Ui::DBTEditor *m_ui;
    ProxyChoiceDecorModel *m_proxyModel;
    QDataWidgetMapper *m_mapper;
};

#endif // DBT_EDITOR_H
