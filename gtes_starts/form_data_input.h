#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

namespace Ui {
    class FormDataInput;
}
class DBTEditor;
class DBTInfo;
class QSqlRelationalTableModel;
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
    void sigSaveData();
    void sigWrongIdEntered();

private slots:
    void slotNeedChangeMapperIndex();
    void slotSubmit();
    void slotEditDBT();
    void slotTemp();

private:
    void setEditDBTPushButtons();
    void setEditDBTOnePB(PBtnForEditDBT *pb, const QString &pbname, QWidget *identWidget);
    void populateData();
    void setRecordsNavigation();
    DBTEditor * createDBTEditor(DBTInfo *info);

    Ui::FormDataInput *ui;
    QSqlRelationalTableModel *m_enginesModel;
    QDataWidgetMapper *m_mapper;
};

#endif // FORM_DATA_INPUT_H
