#ifndef STORAGE_GEN_DATA_H
#define STORAGE_GEN_DATA_H

#include <QVector>
#include <QMap>
#include <QVariant>
#include "../common/common_defines.h"

class StorageGenData
{
public:
    typedef cmmn::T_id T_id; // TODO: replace all T_id to the cmmn::T_id in this class and in the others classes, that uses cmmn::T_id-type value
    typedef QVariant T_data;
    typedef QMap<T_id, QVector<T_data>> T_dataStorage;
    typedef int T_fIndex;
    typedef QVector<T_fIndex> T_fieldsIndexes;

    enum { INIT_FINDEX = 0 };

    StorageGenData();
    void addData(StorageGenData::T_id idPrim, const StorageGenData::T_data &data = StorageGenData::T_data());
    bool deleteData(StorageGenData::T_id idPrim);
    void updateData(StorageGenData::T_id idPrim, int index, const StorageGenData::T_data &data);
    const StorageGenData::T_data & data(StorageGenData::T_id idPrim, int index) const;

    /* Checking - is the DBT field related with the another field by the index "fieldIndex"
     * The refStorageDataIndex is the serial index to the data in the storage */
    bool isForeignField(int fieldIndex, int &refStorageDataIndex) const;
    void addFieldIndex(int colNumb);
    bool hasNextFieldIndex() const;
    StorageGenData::T_fIndex nextFieldIndex();
    void flushFieldIndex();

    bool isEmpty() const;
    bool isNotSetted() const;
    bool isStorageContainsId(StorageGenData::T_id id) const;
    void clear();

private:
    bool isIndexesOk(StorageGenData::T_id id, int index) const;

    T_dataStorage m_storage; // key - the primary id, value - vector of generated data for every foreign key
    T_fieldsIndexes m_fIndexes; // indexes of the main DBT fields, that is foreign keys
    T_fIndex m_indexFIndexes; // index value for going by the fields indexes array
};

#endif // STORAGE_GEN_DATA_H
