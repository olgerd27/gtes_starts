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
    void setDataWithSavings();
    void setTable(const QString &tableName);
    QVariant data(const QModelIndex &item, int role) const;
    bool setData(const QModelIndex &item, const QVariant &value, int role);
    void printData() const; // NOTE: temporary function, delete

public slots:
    void slotRefreshTheModel();
    void slotInsertToTheStorage(int idPrim);
    void slotUpdateTheStorage(int idPrim, int colNumb);

private:
    void defineSimpleDBTAndComplexIndex();
    void fillTheStorage();
    void updateDataInStorage(const QModelIndex &index, int storageComplexIndex);
    void flush();
    void saveData(const QModelIndex &currentIndex, int role);
    void restoreData(int currentRow, int role);
    void fillGeneratedData();

    GeneratorDBTData *m_dataGenerator;
    StorageGenData *m_genDataStorage;
    T_saveRestore m_saveRestore;
    bool m_bNeedSave;
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
