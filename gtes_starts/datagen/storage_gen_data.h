#ifndef STORAGE_GEN_DATA_H
#define STORAGE_GEN_DATA_H

#include <QVector>
#include <QMap>
#include <QVariant>

class StorageGenData
{
public:
    typedef QVariant T_data;
    typedef QMap<int, QVector<T_data>> T_dataStorage;
    typedef int T_fIndex;
    typedef QVector<T_fIndex> T_fieldsIndexes;

    enum { INIT_FINDEX = 0 };

    StorageGenData();
    void addData(int idPrim, const StorageGenData::T_data &data);
    void updateData(int idPrim, int index, const StorageGenData::T_data &data);
    const StorageGenData::T_data & data(int id, int index) const;

    /* Checking - is the DBT foreign field related with the complex DBT by the index "fieldIndex"
     * The refStorageDataIndex is the serial index to the data in the storage */
    bool isComplexDBTField(int fieldIndex, int &refStorageDataIndex) const;
    void addFieldIndex(int colNumb);
    bool hasNextFieldIndex() const;
    StorageGenData::T_fIndex nextFieldIndex();
    void flushFieldIndex();

    bool isEmpty() const;
    bool isNotSetted() const;
    bool isStorageContainsId(int id) const;
    void clear();

private:
    bool isIndexesOk(int id, int index) const;

    T_dataStorage m_storage; // key - the primary id, value - vector of generated data for every foreign key that is related with complex DBT's
    T_fieldsIndexes m_fIndexes; // indexes of the main DBT fields, that is foreign keys and related with complex DBT's
    T_fIndex m_indexFIndexes; // index value for going by the fields indexes array
};

#endif // STORAGE_GEN_DATA_H
