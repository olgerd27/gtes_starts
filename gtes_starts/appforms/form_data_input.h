#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>
#include <memory>

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
class ChangerMChTypeImpl;

/*
 * Class for change model change type. The available types are defined in the MChTypeLabel class.
 */
class ChangerMChType : public QObject
{
    Q_OBJECT

public:
    explicit ChangerMChType(QObject *parent = 0);
    ~ChangerMChType();
    void updateModelChange(const QVariant &idPrimary, int changeType); // changeType is the MChTypeLabel::ChangeTypes
    void clearChanges();

public slots:
    void slotCheckModelChanges(const QVariant &idPrimary);

signals:
    void sigChangeChangedType(int ctype);
    void sigChangeChangedType(bool isDeleted);

private:
    std::unique_ptr<ChangerMChTypeImpl> m_pImpl; // for using std::unique_ptr<> there are need to implement the ChangerMChType destructor
};

/*
 * The form for data inputing in the database
 */
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
//    void sigRevertChanges();
    void sigWrongIdEntered();
    void sigFirstRowReached(bool);
    void sigLastRowReached(bool);
    void sigChangesSubmitted(int currentIndex);

public slots:
    void slotDeleteRow();

private slots:
    void slotNeedChangeMapperIndex(const QString &value);
    void slotRowIndexChanged(int row);
    void slotSubmit();
    void slotEditDBT();

private:
    void setMainControls();
    void setEditDBTPushButtons();
    void setEditDBTOnePB(PBtnForEditDBT *pb, const QString &pbname, QWidget *identWidget);
    void setDataOperating();
    void setDataNavigation();
    void setModelChange();

    DBTEditor * createDBTEditor(dbi::DBTInfo *info);

    std::unique_ptr<Ui::FormDataInput> ui;
    std::unique_ptr<CustomSqlTableModel> m_enginesModel;
    std::unique_ptr<QDataWidgetMapper> m_mapper;
    std::unique_ptr<ChangerMChType> m_mchTChanger;
};



#endif // FORM_DATA_INPUT_H
