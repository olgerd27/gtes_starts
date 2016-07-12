#include <QDebug> // TODO: delete later
#include "storage_gen_data.h"

StorageGenData::StorageGenData()
    : m_indexFIndexes(INIT_FINDEX)
{ }

void StorageGenData::addData(T_id idPrim, const StorageGenData::T_data &data)
{
    /*
     * Add data "data" to the storage with the primary id "idPrim".
     * If data with "idPrim" does not exist, the map create item with key "idPrim" and operator[] create the empty vector instance
     * (with help of the vector<StorageGenData::T_data> default constructor) as current map item value.
     * After this the "data" value will push to the back of it.
     *
     * If user does not pass "data" value as 2-th argument, it will be created with help of the StorageGenData::T_data default constructor.
     * This behaviour is setted in definition of this method as default argument value.
     */
    m_storage[idPrim].push_back(data);
}

bool StorageGenData::deleteData(StorageGenData::T_id idPrim)
{
    qDebug() << "delete data: idPrim = " << idPrim << ", data:";
    for (auto val : m_storage[idPrim])
        qDebug() << "  " << val;
    return m_storage.remove(idPrim);
}

void StorageGenData::updateData(T_id idPrim, int index, const StorageGenData::T_data &data)
{
//    qDebug() << "update generated data. id prim:" << idPrim << ", index:" << index << ", data:" << data;

    // TODO: print all stored data - delete later
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
                QObject::tr("Cannot update data in the generated data storage by the primary id = %1, index = %2. "
                            "Invalid id and/or index values").arg(idPrim).arg(index),
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

void StorageGenData::clearForeignFields()
{
    m_fIndexes.clear();
    flushFieldIndex();
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

bool StorageGenData::isNotSettedForeignFields() const
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
