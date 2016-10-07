#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <memory>
#include <QDialog>
#include "../common/common_defines.h"

/*
 * The class for editing database tables (DBT)
 */
namespace Ui { class DBTEditor; }
namespace dbi { class DBTInfo; }
class ProxyDecorModel;
class ProxyFilterModel;
class EditUICreator;
class WidgetMapper;
class DBTEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent = 0);
    ~DBTEditor();

    void selectInitial(const QVariant &idPrim);
    cmmn::T_id selectedId() const;

signals:
    void sigDataSavedInDB();

public slots:
    void accept();

private slots:
    void slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo);

private:
    enum SETTINGS {
          shiftCEByX = 50 // shift appearance position of a created child editor by X
        , shiftCEByY = 50 // shift appearance position of a created child editor by Y
    };

    void setWindowPosition();
    void setWindowName();
    void setModel();
    void setMapper();
    void setSelectUI();

    void setFilter();
    void setEditUI();
    void setMainControl();
    void setDataNavigation();

    void askSaving();

    // TODO: use the std::unique_ptr after debugging
    Ui::DBTEditor *m_ui;
    const dbi::DBTInfo *m_DBTInfo;
    ProxyDecorModel *m_prxDecorMdl_1;
    ProxyFilterModel *m_prxFilterMdl_2;
    WidgetMapper *m_mapper;
    std::unique_ptr<EditUICreator> m_editUICreator;
    int m_initSelectRow;
};

#endif // DBT_EDITOR_H
