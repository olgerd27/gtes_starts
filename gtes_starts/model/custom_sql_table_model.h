#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <memory>
#include "../common/common_defines.h"

class GeneratorDBTData;
class StorageGenData;

class CustomSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    typedef QMap<int, QVariant> T_saveRestore;
    enum { SELECT_ICON_COLUMN = 0 };

    explicit CustomSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    ~CustomSqlTableModel();

    void setTable(const QString &tableName);
    QVariant data(const QModelIndex &idx, int role) const;
    bool setData(const QModelIndex &idx, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QVariant primaryIdInRow(int row) const;
    bool findPrimaryIdRow(const QVariant &idPrim, int &rRowValue) const;
    cmmn::T_id selectedId() const;

    void spike1_turnOn(bool bOn); // spike 1

    void printDataBase(int role) const; // TODO: temporary function, delete later
    void printDataCustom(int role) const; // TODO: temporary function, delete later
    QString printRecords() const; // TODO: temporary function, delete later
    void printHeader(int role) const; // TODO: temporary function, delete later

signals:
    void sigNewRecordInserted(int row, cmmn::T_id primaryId);
    void sigRecordDeleted(int row, cmmn::T_id primaryId);
    void sigModelRefreshed();
    void sigNeedUpdateView(const QModelIndex &index);

public slots:
    void slotRefreshTheModel();
    void slotInsertToTheModel();
    void slotDeleteFromTheModel(int row);
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    enum { NOT_SETTED = -1 };

    void setComplexIdx();
    void defineSimpleDBTAndHeader();
    void fillTheStorage();
    cmmn::T_id insertNewRecord();
    QVariant getDataFromStorage(const QModelIndex &baseIndex, int storageComplexIndex) const;
    void updateDataInStorage(const QModelIndex &index, int storageComplexIndex);
    void flush();
    void spike1_saveData(const QModelIndex &modelIndex); // spike 1
    void spike1_restoreData(const QModelIndex &modelIndex); // spike 1
    void fillGeneratedData();

    std::unique_ptr<GeneratorDBTData> m_dataGenerator;
    std::unique_ptr<StorageGenData> m_genDataStorage;
    int m_selectedRow;
    QIcon m_selectIcon;
    bool m_spike1_bNeedSave; // spike 1
    T_saveRestore m_spike1_saveRestore; /* spike 1 TODO: move this storage to the StorageGenData and provide in the StorageGenData
                                         * the interface for getting data from this storage */
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
