#include <QDebug> // TODO: delete later
#include "storage_gen_data.h"

StorageGenData::StorageGenData()
    : m_indexFIndexes(INIT_FINDEX)
{
}

void StorageGenData::addData(T_id idPrim, const StorageGenData::T_data &data)
{
    /*
     * Add data to the storage with the primary id "idPrim".
     * If data with "idPrim" does not exist, operator[] create vector (with help of the vector<StorageGenData::T_data> default constructor)
     * and assign the "data" value to it.
     * If user does not pass "data" value as 2-th argument, it will be create with help of the StorageGenData::T_data default constructor.
     * This behaviour is setted in definition of this method.
     */
    m_storage[idPrim].push_back(data);
}

bool StorageGenData::deleteData(StorageGenData::T_id idPrim)
{
    return m_storage.remove(idPrim);
}

void StorageGenData::updateData(T_id idPrim, int index, const StorageGenData::T_data &data)
{
//    qDebug() << "update generated data. id prim:" << idPrim << ", index:" << index << ", data:" << data;
    if ( !isIndexesOk(idPrim, index) ) {
        throw std::out_of_range(
                QObject::tr("Cannot update the data in the storage by the primary id = %1, index = %2. This id and/or index is invalid")
                .arg(idPrim).arg(index).toStdString() );
    }
    m_storage[idPrim][index] = data;
}

const StorageGenData::T_data & StorageGenData::data(T_id id, int index) const
{
    if ( !isIndexesOk(id, index) ) {
        throw std::out_of_range(
                QString("Cannot return the data from the generated data storage by the id = %1, index = %2")
                .arg(id).arg(index).toStdString() );
    }
    return m_storage[id][index];
}

void StorageGenData::addFieldIndex(int colNumb)
{
    m_fIndexes.push_back(colNumb);
}

bool StorageGenData::hasNextFieldIndex() const
{
    return m_indexFIndexes < m_fIndexes.size();
}

StorageGenData::T_fIndex StorageGenData::nextFieldIndex()
{
    return m_fIndexes.at(m_indexFIndexes++);
}

void StorageGenData::flushFieldIndex()
{
    m_indexFIndexes = INIT_FINDEX;
}

bool StorageGenData::isComplexDBTField(int fieldIndex, int &refStorageDataIndex) const
{
    refStorageDataIndex = m_fIndexes.indexOf(fieldIndex);
    return refStorageDataIndex != -1;
}

bool StorageGenData::isEmpty() const
{
    return m_storage.empty();
}

bool StorageGenData::isNotSetted() const
{
    return m_fIndexes.empty();
}

bool StorageGenData::isStorageContainsId(T_id id) const
{
    return m_storage.contains(id);
}

void StorageGenData::clear()
{
    m_storage.clear();
}

bool StorageGenData::isIndexesOk(T_id id, int index) const
{
    return isStorageContainsId(id) && (index >= 0) && (index < m_storage[id].size());
}
