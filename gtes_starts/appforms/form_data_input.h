#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

namespace Ui {
    class FormDataInput;
}
namespace dbi {
    class DBTInfo;
}
class DBTEditor;
class CustomSqlTableModel;
class QDataWidgetMapper;
class PBtnForEditDBT;

class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

signals:
    void sigChangeMapperIndex(int index);
    void sigInsertNew();
    void sigDeleteRow(int row);
    void sigSaveAll();
    void sigRefreshAll();
    void sigWrongIdEntered();
    void sigFirstRowReached(bool);
    void sigLastRowReached(bool);

public slots:
    void slotDeleteRow();

private slots:
    void slotNeedChangeMapperIndex();
    void slotCheckExtremeRowsReaching(int row);
    void slotSubmit();
    void slotEditDBT();

private:
    void setEditDBTPushButtons();
    void setEditDBTOnePB(PBtnForEditDBT *pb, const QString &pbname, QWidget *identWidget);
    void setDataOperating();
    void setDataNavigation();
    DBTEditor * createDBTEditor(dbi::DBTInfo *info);

    Ui::FormDataInput *ui;
    CustomSqlTableModel *m_enginesModel;
    QDataWidgetMapper *m_mapper;
};

#endif // FORM_DATA_INPUT_H
