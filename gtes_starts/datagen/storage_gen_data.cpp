#include <QDebug> // TODO: delete
#include "storage_gen_data.h"

StorageGenData::StorageGenData()
    : m_indexFIndexes(INIT_FINDEX)
{
}

void StorageGenData::addData(int idPrim, const StorageGenData::T_data &data)
{
    // if a data with this "id" is not exist, operator[] create it with help of the StorageGenData::T_data default constructor and assign the "data" value to it
    m_storage[idPrim].push_back(data);
}

void StorageGenData::updateData(int idPrim, int index, const StorageGenData::T_data &data)
{
    qDebug() << "update generated data. id prim:" << idPrim << ", index:" << index << ", data:" << data;
    if ( !isIndexesOk(idPrim, index) ) {
        throw std::out_of_range(
                QString("Cannot return the data from the generated data storage by the primary id = %1, index = %2")
                .arg(idPrim).arg(index).toStdString() );
    }
    m_storage[idPrim][index] = data;
//    qDebug() << "update data in the storage, id:" << idPrim << ", index:" << index << ", data:" << data.toString();
}

StorageGenData::T_data StorageGenData::data(int id, int index)
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

bool StorageGenData::isComplexDBTFieldIndex(int fieldIndex, int &refStorageDataIndex) const
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

bool StorageGenData::isStorageContainsId(int id) const
{
    return m_storage.contains(id);
}

void StorageGenData::clear()
{
    m_storage.clear();
}

bool StorageGenData::isIndexesOk(int id, int index) const
{
    return isStorageContainsId(id) && (index >= 0) && (index < m_storage[id].size());
}
