#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>
#include <memory>

/*
 * Class for change model change type.
 * The available types are defined in the MChTypeLabel class, inherited from the QLabel.
 */
class ChangerMChTypeImpl;
class ChangerMChType : public QObject
{
    Q_OBJECT

public:
    explicit ChangerMChType(QObject *parent = 0);
    ~ChangerMChType();
    void updateModelChange(const QVariant &idPrimary, int changeType); // changeType is the MChTypeLabel::ChangeTypes

public slots:
    void slotCheckModelChanges(const QVariant &idPrimary);
    void slotClearChanges();

signals:
    void sigChangeChangedType(int ctype);
    void sigChangeChangedType(bool isDeleted);

private:
    std::unique_ptr<ChangerMChTypeImpl> m_pImpl; // std::unique_ptr<> needs ChangerMChType destructor implementation
};

/*
 * The form for data input in the database
 */
namespace Ui { class FormDataInput; }
namespace dbi { class DBTInfo; }
class ProxyDecorModel;
class WidgetMapper;
class SelectEditPB; // TODO: delete after adding the EditUICreator for creating interface
class EditUICreator;
class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

signals:
    void sigChangeMapperIndex(int index);
    void sigInsertNew();
    void sigDeleteRow();
    void sigSaveAll();
    void sigRefreshAll();
//    void sigRevertChanges();
    void sigWrongIdEntered();
    void sigFirstRowReached(bool);
    void sigLastRowReached(bool);
    void sigEngineNameGenerated(const QString &engName);

private slots:
    void slotNeedChangeMapperIndex(const QString &value);
    void slotCheckRowIndex(int row);
    void slotGenEngineName(int row);
    void slotEditChildDBT();
    void slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo);

private:
    void setModel();
    void setMapper();
    void setEditUI();
    void setMainControls();
//    void setEditDBTPushButtons(); // delete
//    void setEditDBTOnePB(SelectEditPB *pb, const QString &pbname, QWidget *identWidget); // delete
//    void setDataOperating(); // delete
    void setDataNavigation();
    void setModelChange();
    void setEngineName();

    // TODO: use the std::unique_ptr after debugging
    Ui::FormDataInput *m_ui;
    const dbi::DBTInfo *m_DBTInfo;
    ProxyDecorModel *m_prxDecorMdl;
    WidgetMapper *m_mapper;
    EditUICreator *m_editUICreator;
    ChangerMChType *m_mchTChanger;
};

#endif // FORM_DATA_INPUT_H
