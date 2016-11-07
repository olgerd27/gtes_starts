#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>
#include <memory>

namespace dbi { class DBTInfo; }

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
    void sigChangeChType(int chtype);

private:
    std::unique_ptr<ChangerMChTypeImpl> m_pImpl; // std::unique_ptr<> needs ChangerMChType destructor implementation
};

/*
 * EDGenerator - entity data generator. Generate data of any database entity, for example the "engine name" data.
 */
class QAbstractTableModel;
class EDGenerator : public QObject
{
    Q_OBJECT
public:
    EDGenerator(const QAbstractTableModel *m, const dbi::DBTInfo *info);
    inline void setDataEmpty(bool b) { m_isDataEmpty = b; }
signals:
    void sigGenerated(const QString &data);
public slots:
    void slotGenerate(int dbtRow);
private:
    const QAbstractTableModel *m_model;
    const dbi::DBTInfo *m_dbtInfo;
    bool m_isDataEmpty;
};

/*
 * The form for data input in the database
 */
namespace Ui { class FormDataInput; }
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
    void sigCurrentIdChanged(const QString &id);
    void sigWrongIdEntered();
    void sigFirstRowReached(bool);
    void sigLastRowReached(bool);

private slots:
    void slotNeedChangeMapperIndex(const QString &value);
    void slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo);

private:
    void setModel();
    void setMapper();
    void setEditUI();
    void setMainControls();
    void setDataNavigation();
    void setLEid();
    void setModelChange();
    void setEngineName();

    // TODO: use the std::unique_ptr after debugging
    Ui::FormDataInput *m_ui;
    const dbi::DBTInfo *m_DBTInfo;
    ProxyDecorModel *m_prxDecorMdl;
    WidgetMapper *m_mapper;
    EditUICreator *m_editUICreator;
    ChangerMChType *m_mchTChanger;
    EDGenerator *m_edgen;
};

#endif // FORM_DATA_INPUT_H
