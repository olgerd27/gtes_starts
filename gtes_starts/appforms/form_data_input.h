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
    void sigSaveAll();
    void sigRefreshAll();
    void sigWrongIdEntered();

private slots:
    void slotNeedChangeMapperIndex();
    void slotSubmit();
    void slotEditDBT();

private:
    void setEditDBTPushButtons();
    void setEditDBTOnePB(PBtnForEditDBT *pb, const QString &pbname, QWidget *identWidget);
    void setDataOperating();
    void setModel();
    void setDataNavigation();
    DBTEditor * createDBTEditor(dbi::DBTInfo *info);

    Ui::FormDataInput *ui;
    CustomSqlTableModel *m_enginesModel;
    QDataWidgetMapper *m_mapper;
};

#endif // FORM_DATA_INPUT_H
