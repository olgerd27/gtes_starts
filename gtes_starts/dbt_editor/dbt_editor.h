#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <memory>
#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>
#include <QItemSelection>
#include <QDataWidgetMapper>
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
class EditUICreator;
class WidgetDataSender;
class DBTEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent = 0);
    ~DBTEditor();

    void selectInitial(const QVariant &idPrim);
    cmmn::T_id selectedId() const;

public slots:
    void slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo);

private slots:
    void slotFocusLost_DataSet(QWidget *w, const QString &data);

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
    std::unique_ptr<WidgetDataSender> m_transmitter;
    std::unique_ptr<EditUICreator> m_editUICreator;
};

#endif // DBT_EDITOR_H
