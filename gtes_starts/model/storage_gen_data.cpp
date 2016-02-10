#include <QDebug> // TODO: delete later
#include "storage_gen_data.h"

StorageGenData::StorageGenData()
    : m_indexFIndexes(INIT_FINDEX)
{ }

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
    QString str;
    for (auto itId = m_storage.begin(); itId != m_storage.end(); ++itId) {
        str += (QString::number(itId.key()) + ": ");
        for (auto itIdx = itId->begin(); itIdx != itId->end(); ++itIdx) {
            str += "[";
            str += ((*itIdx).toString() + "]");
        }
        str += "\n";
    }
    qDebug() << str;
    ASSERT_DBG( isIndexesOk(idPrim, index),
                cmmn::MessageException::type_critical, QObject::tr("Data update error"),
                QObject::tr("Cannot update data in the storage by the primary id = %1, index = %2. "
                            "This id and/or index is invalid").arg(idPrim).arg(index),
                QString("StorageGenData::updateData()") );
    m_storage[idPrim][index] = data;
}

const StorageGenData::T_data & StorageGenData::data(T_id idPrim, int index) const
{
    ASSERT_DBG( isIndexesOk(idPrim, index),
                cmmn::MessageException::type_critical, QObject::tr("Data update error"),
                QObject::tr("Cannot return the data from the generated data storage by the id = %1, index = %2."),
                QString("StorageGenData::data()") );
    return m_storage[idPrim][index];
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

bool StorageGenData::isForeignField(int fieldIndex, int &refStorageDataIndex) const
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
