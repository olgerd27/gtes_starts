#include <QDebug> // TODO: delete
#include "gen_data_storage.h"

GenDataStorage::GenDataStorage()
    : m_indexFIndexes(INIT_FINDEX)
{
}

void GenDataStorage::addData(int idPrim, const GenDataStorage::T_data &data)
{
    // if a data with this "id" is not exist, operator[] create it with help of the GenDataStorage::T_data default constructor and assign the "data" value to it
    m_storage[idPrim].push_back(data);
}

void GenDataStorage::updateData(int idPrim, int index, const GenDataStorage::T_data &data)
{
    if ( !isIndexesOk(idPrim, index) ) {
        throw std::out_of_range(
                QString("Cannot return the data from the generated data storage by the primary id = %1, index = %2")
                .arg(idPrim).arg(index).toStdString() );
    }
    m_storage[idPrim][index] = data;
//    qDebug() << "update data in the storage, id:" << idPrim << ", index:" << index << ", data:" << data.toString();
}

GenDataStorage::T_data GenDataStorage::data(int id, int index)
{
    if ( !isIndexesOk(id, index) ) {
        throw std::out_of_range(
                QString("Cannot return the data from the generated data storage by the id = %1, index = %2")
                .arg(id).arg(index).toStdString() );
    }
    return m_storage[id][index];
}

void GenDataStorage::addFieldIndex(int colNumb)
{
    m_fIndexes.push_back(colNumb);
}

bool GenDataStorage::hasNextFieldIndex() const
{
    return m_indexFIndexes < m_fIndexes.size();
}

GenDataStorage::T_fIndex GenDataStorage::nextFieldIndex()
{
    return m_fIndexes.at(m_indexFIndexes++);
}

void GenDataStorage::flushToGetFIndex()
{
    m_indexFIndexes = INIT_FINDEX;
}

bool GenDataStorage::isComplexDBTFieldIndex(int fieldIndex, int &refStorageDataIndex) const
{
    refStorageDataIndex = m_fIndexes.indexOf(fieldIndex);
    return refStorageDataIndex != -1;
}

bool GenDataStorage::isEmpty() const
{
    return m_storage.empty();
}

bool GenDataStorage::isNotSetted() const
{
    return m_fIndexes.empty();
}

bool GenDataStorage::isStorageContainsId(int id) const
{
    return m_storage.contains(id);
}

void GenDataStorage::clear()
{
    m_storage.clear();
}

bool GenDataStorage::isIndexesOk(int id, int index) const
{
    return isStorageContainsId(id) && (index >= 0) && (index < m_storage[id].size());
}
