#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>

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
    bool setData(const QModelIndex &item, const QVariant &value, int role);
    void spike1_turnOn(bool bOn);
    void printData(int role) const; // NOTE: temporary function, delete

public slots:
    void slotRefreshTheModel();
    void slotInsertToTheStorage(int idPrim);
    void slotUpdateTheStorage(int idPrim, int colNumb);

private:
    void defineSimpleDBTAndComplexIndex();
    void fillTheStorage();
    void getDataFromStorage(QVariant &data, const QModelIndex &index, int storageComplexIndex) const;
    void updateDataInStorage(const QModelIndex &index, int storageComplexIndex);
    void flush();
    void saveData_spike1(const QModelIndex &currIndex);
    void restoreData_spike1(int currRow);
    void fillGeneratedData();

    GeneratorDBTData *m_dataGenerator;
    StorageGenData *m_genDataStorage;
    T_saveRestore m_saveRestore;
    bool m_bNeedSave_spike1;
};

class CustomSqlRelationalDelegate : public QSqlRelationalDelegate
{
public:
    CustomSqlRelationalDelegate(QObject *parent);
    ~CustomSqlRelationalDelegate();
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
private:
    void setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // CUSTOM_SQL_TABLE_MODEL_H
