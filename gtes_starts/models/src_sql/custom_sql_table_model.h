#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <memory>
#include "../../common/common_defines.h"

// The main model, that perform interactions with DB
class GeneratorDBTData;
class StorageGenData;
class Spike1;
class CustomSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
    friend class Spike1;

public:
    typedef QMap<int, QVariant> T_saveRestore;

    explicit CustomSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~CustomSqlTableModel();

    void setTable(const QString &tableName);
    void setHeader();
    QVariant data(const QModelIndex &idx, int role) const;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole);

    QVariant primaryIdInRow(int row) const;
    bool getIdRow(const QVariant &idPrim, int &rRowValue) const;

    void spike1_turnOn(); // spike 1

    void printDataDB(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later
    void printHeader(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later

signals:
    void sigNewRecordInserted(int row, cmmn::T_id primaryId);
    void sigRecordDeleted(int row, cmmn::T_id primaryId);
    void sigModelRefreshed();
    void sigSavedInDB();

public slots:
    void slotRefreshTheModel();
    void slotInsertToTheModel();
    void slotDeleteRowRecord(int row);
    void slotSaveToDB();

private:
    enum { NOT_SETTED = -1 };

    void defineForeignFields();
    void fillTheStorage();
    cmmn::T_id insertNewRecord();
    QVariant getDataFromStorage(const QModelIndex &index, int storageComplexIndex) const;
    void updateDataInStorage(const QModelIndex &frgnIndex, int storageComplexIndex);
    void flushGenData();
    void setParentData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole); // spike 1

    // TODO: use std::unique_ptr after debugging
    //std::unique_ptr<GeneratorDBTData> m_dataGenerator;
    //std::unique_ptr<StorageGenData> m_genDataStorage;
    GeneratorDBTData *m_dataGenerator;
    StorageGenData *m_genDataStorage;
    Spike1 *m_spike1;

    T_saveRestore m_spike1_saveRestore;
};

#endif // CUSTOM_SQL_TABLE_MODEL_H
