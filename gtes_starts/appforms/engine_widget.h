#ifndef ENGINE_IN_WIDGET_H
#define ENGINE_IN_WIDGET_H

#include <memory>
#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include "../common/common_defines.h"

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

public slots:
    void slotUpdateChange(const cmmn::T_id &idPrimary, int chType);
    void slotClearChanges();
    void slotCheckModelChanges(const QVariant &idPrimary);

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

signals:
    void sigGenerated(const QString &data);

public slots:
    void slotSetDataEmpty(bool b) { m_isDataEmpty = b; }
    void slotGenerate(int row);

private:
    const QAbstractTableModel *m_model;
    const dbi::DBTInfo *m_dbtInfo;
    bool m_isDataEmpty;
};

/*
 * The widget with engine's data
 */
class ProxyDecorModel;
class WidgetMapper;
class EditUICreator;
class MChTypeLabel;
class EngineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EngineWidget(QWidget *parent = 0);
    void init();

signals:
    // make operations with data changing
    void sigInsertNew();
    void sigDeleteRow();
    void sigSaveAll();
    void sigRefreshAll();

    // notifications about end of operations with data changing
    void sigRecordInserted(cmmn::T_id primId);
    void sigRecordDeleted(cmmn::T_id primId);
    void sigDataSaved();
    void sigDataRefreshed();

    // make data navigation
    void sigToFirstRec();
    void sigToLastRec();
    void sigToPreviousRec();
    void sigToNextRec();

    // data navigation - results
    void sigFirstRecReached(bool);
    void sigLastRecReached(bool);
    void sigCurrentIdChanged(const QString &id);
    void sigWrongEngineId();

    void sigChTypeChanged(int);
    void sigChTypeToDeletedChanged(bool);
    void sigEngineNameGenerated(QString);

public slots:
    void slotTryChangeEngine(const QString &id);

private slots:
    void slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo);

private:
    void setModel();
    void setMapper();
    void setUI();
    void setMainControls();
    void setDataNavigation();
    void setModelChange();
    void setEngineName();

    // TODO: use the std::unique_ptr after debugging
    const dbi::DBTInfo *m_DBTInfo;
    ProxyDecorModel *m_prxDecorMdl;
    WidgetMapper *m_mapper;
    QGroupBox *m_grBoxAll;
    EditUICreator *m_editUICreator;
    ChangerMChType *m_mchTChanger;
    EDGenerator *m_engNameGen;
};

#endif // ENGINE_IN_WIDGET_H
