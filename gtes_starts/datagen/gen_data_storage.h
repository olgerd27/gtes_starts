#ifndef GEN_DATA_STORAGE_H
#define GEN_DATA_STORAGE_H

#include <QVector>
#include <QMap>
#include <QVariant>

class GenDataStorage
{
public:
    typedef QVariant T_data;
    typedef QMap<int, QVector<T_data>> T_dataStorage;
    typedef int T_fIndex;
    typedef QVector<T_fIndex> T_fieldsIndexes;

    enum { INIT_FINDEX = 0 };

    GenDataStorage();
    void addData(int idPrim, const GenDataStorage::T_data &data);
    void updateData(int idPrim, int index, const GenDataStorage::T_data &data);
    GenDataStorage::T_data data(int id, int index);

    /* Checking - is the DBT field related to the complex DBT by the index "fieldIndex"
     * The refStorageDataIndex is the serial index to the data in the storage */
    bool isComplexDBTFieldIndex(int fieldIndex, int &refStorageDataIndex) const;
    void addFieldIndex(int colNumb);
    bool hasNextFieldIndex() const;
    GenDataStorage::T_fIndex nextFieldIndex();
    void flushToGetFIndex();

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

#endif // GEN_DATA_STORAGE_H
