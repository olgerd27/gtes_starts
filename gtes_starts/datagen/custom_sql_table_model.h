#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include "../common/common_defines.h"

class GeneratorDBTData;
class StorageGenData;

class CustomSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    typedef QMap<int, QVariant> T_saveRestore;

    explicit CustomSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~CustomSqlTableModel();
    void setTable(const QString &tableName);
    QVariant data(const QModelIndex &item, int role) const;
    bool setData(const QModelIndex &item, const QVariant &value, int role = Qt::EditRole);
    QVariant primaryIdInRow(int row) const;
    bool findPrimaryId(const QVariant id, int &row);
    void spike1_turnOn(bool bOn);
    void printData(int role) const; // TODO: temporary function, delete later
    QString printRecords() const; // TODO: temporary function, delete later

signals:
    void sigNewRecordInserted(int row, cmmn::T_id primaryId);
    void sigRecordDeleted(int row, cmmn::T_id primaryId);
    void sigModelRefreshed();

public slots:
    void slotRefreshTheModel();
    void slotInsertToTheModel();
    void slotDeleteFromTheModel(int row);

private:
    enum { NOT_SETTED = -1 };

    void defineSimpleDBTAndComplexIndex();
    void fillTheStorage();
    cmmn::T_id insertNewRecord();
    void getDataFromStorage(QVariant &data, const QModelIndex &index, int storageComplexIndex) const;
    void updateDataInStorage(const QModelIndex &index, int storageComplexIndex);
    void flush();
    void spike1_saveData(const QModelIndex &currIndex);
    void spike1_restoreData(const QModelIndex &currIndex);
    void fillGeneratedData();

    GeneratorDBTData *m_dataGenerator;
    StorageGenData *m_genDataStorage;
    T_saveRestore m_saveRestore; // TODO: move this storage to the StorageGenData and produce in the StorageGenData the interface for this storage
    bool m_spike1_bNeedSave;
};

class CustomSqlRelationalDelegate : public QSqlRelationalDelegate
{
public:
    explicit CustomSqlRelationalDelegate(QObject *parent);
    ~CustomSqlRelationalDelegate();
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
private:
    void setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // CUSTOM_SQL_TABLE_MODEL_H
