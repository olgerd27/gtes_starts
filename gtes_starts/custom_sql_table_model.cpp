#include <QSqlQuery>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "db_info.h"
#include "common_defines.h"

/*
 * DisplayDataGenerator
 */
class DisplayDataGenerator
{
public:
    QString generate(const QString &tableName, const QVariant &varId);
private:
    /* QueryGenerator */
    class QueryGenerator
    {
    public:
        inline void addSelect(const QString &str) { m_listSelect.push_back(str); }
        inline void addFrom(const QString &str) { m_listFrom.push_back(str); }
        inline void addWhere(const QString &str) { m_listWhere.push_back(str); }
        QString generateQuery(int id); // there are ability to get only one time the particular generated query string
    private:
        void flush();
        QStringList m_listSelect, m_listFrom, m_listWhere;
    };

    int safeIdToInt(const QVariant &value) const;
    void generate_Mask_QueryData(dbi::DBTInfo *table, QString &data, int &fieldCounter);
    void generateResultData(int id, QString &strRes, int fieldsNumber, const QString &tableName);

    QueryGenerator m_queryGen;
};

QString DisplayDataGenerator::generate(const QString &tableName, const QVariant &varId)
{
    QString strRes("");
    try {
        int fieldsCounter = 0;
        generate_Mask_QueryData( DBINFO.tableByName(tableName), strRes, fieldsCounter );
//        qDebug() << "mask:" << strRes;
        generateResultData( safeIdToInt(varId), strRes, fieldsCounter, tableName );
    }
    catch (const cmmn::MessageException &me) {
        if (me.type() == cmmn::MessageException::type_critical)
            strRes.clear();
        qDebug() << "[ERROR]: Title:" << me.title() << "| Message:" << me.message() << "| Place:" << me.placement();
//        QString("\n%1: [%2]").arg(tr("Error placement")).arg(me.placement()); // TODO: add this info to main message and pass all to the message box
        return strRes;
    }
    catch (const std::exception &ex) {
        strRes.clear();
        return strRes;
    }
//    qDebug() << "result:" << strRes;
    return strRes;
}

/* Generate data mask and data for generation query string */
void DisplayDataGenerator::generate_Mask_QueryData(dbi::DBTInfo *table, QString &data, int &fieldCounter)
{
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        const dbi::DBTFieldInfo &field = table->fieldByIndex( idnField.m_NField );
        data += idnField.m_strBefore;
        if (field.isForeign()) {
            m_queryGen.addWhere( table->m_nameInDB + ".id" );
            m_queryGen.addWhere( table->m_nameInDB + "." + field.m_nameInDB );
            generate_Mask_QueryData( DBINFO.tableByName(field.m_relationDBTable), data, fieldCounter ); /* recursive calling */
        }
        else {
            data += QString("%%1").arg(++fieldCounter); /* when current field isn't a foreign key -> exit from recursion */
            m_queryGen.addSelect( table->m_nameInDB + "." + field.m_nameInDB );
            m_queryGen.addFrom( table->m_nameInDB );
        }
    }
}

int DisplayDataGenerator::safeIdToInt(const QVariant &value) const
{
    bool b = false;
    int id = value.toInt(&b);
    if (!b)
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the foreign key value \"%1\" from the QVariant to the integer type.")
                                      .arg(value.toString()), "DisplayDataGenerator::safeIdToInt");
    return id;
}

/* Run query for retriving data from relational DB tables and generate result string data for displaying */
void DisplayDataGenerator::generateResultData(int id, QString &strRes, int fieldsNumber, const QString &tableName)
{
    QString strQuery = m_queryGen.generateQuery(id);
//    qDebug() << "query:" << strQuery;
    QSqlQuery query(strQuery);
    if (query.next()) {
        for (int field = 0; field < fieldsNumber; field++)
            strRes = strRes.arg( query.value(field).toString() );
    }
    else {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error data getting"),
                                      QObject::tr("Cannot get a data from the database for generation displayed data of the \"%1\" database table.\n"
                                                  "The reason is: invalid database query.").arg(tableName), "DisplayDataGenerator::generateResultData" );
    }
//    qDebug() << "strRes data:" << strRes << "for id =" << id;
}

/*
 * DisplayDataGenerator::QueryGenerator
 */
QString DisplayDataGenerator::QueryGenerator::generateQuery(int id)
{
    if (m_listFrom.isEmpty() || m_listSelect.isEmpty()) {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error query generation"),
                                      QObject::tr("Too many attempts to get the query, used for generation displayed data"),
                                      "DisplayDataGenerator::QueryGenerator::generateQuery" );
    }
    /* preparing the FROM statement */
    m_listFrom.removeDuplicates();

    /* preparing the WHERE statements */
    m_listWhere.push_front("%1");
    m_listWhere.push_back(m_listFrom.first() + ".id");
    QString strWhere("");
    for (auto it = m_listWhere.cbegin(); it != m_listWhere.cend(); it += 2) {
        if (it != m_listWhere.cbegin()) strWhere += "AND ";
        strWhere += ( *it + " = " + *(it + 1) + " " );
    }
//    qDebug() << "SELECT:" << m_listSelect.join(", ");
//    qDebug() << "FROM:" << m_listFrom.join(", ");
//    qDebug() << "WHERE:" << strWhere;

    QString strQuery = QString("SELECT %1 FROM %2 WHERE %3;")
            .arg( m_listSelect.join(", ") ).arg( m_listFrom.join(", ") ).arg( strWhere.arg(id) );
    flush();
    return strQuery;
}

void DisplayDataGenerator::QueryGenerator::flush()
{
    m_listSelect.clear();
    m_listFrom.clear();
    m_listWhere.clear();
}

/*
 * CustomSqlTableModel
 * Description of the spike #1.
 * When user change a some field that is a foreign key (in particular, a foreign key of a related complex DB table), there are performs
 * a calling of the QSqlRelationalTableModel::setData() method from the CustomSqlTableModel::setData().
 * The QSqlRelationalTableModel::setData() method (or maybe the QSqlTableModel::setData() method (called from the
 * QSqlRelationalTableModel::setData()) ) set some incorrect data to the items, that is different from present (current item).
 * This setted data are: to the DisplayRole - a QString-type already generated data, to the EditRole - a QString-type empty instance.
 * This invalid behaviour of data settings, which is protected from influent, need to prevent. This is achieved by usage saving and
 * following restoring data, that must not change.
 * Turn on the run of this operations performs by calling the public method setDataWithSavings(). Turn off the run of this operations
 * performs without assistance from outside in the restoreData() method.
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
    , m_bNeedSave(false) /* Spike #1 */
{ }

void CustomSqlTableModel::setDataWithSavings()
{
    m_bNeedSave = true; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    select();
    setRelation(2, QSqlRelation("fuels_types", "id", "name")); // TODO: make an automatic relations set
    fillGeneratedData();
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    if (!item.isValid()) return QVariant();

    QVariant data;
    if (role == Qt::UserRole)
        data = QSqlRelationalTableModel::data(item, role);
    else if (role == Qt::DisplayRole || role == Qt::EditRole)
        data = m_generatedData[]
//    qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();

//    int colNumb = item.column();
//    if ( colNumb != 2 && role == Qt::EditRole ) {
//        dbi::DBTFieldInfo fieldInfo = dbi::fieldByNameIndex(tableName(), colNumb);
//        if (fieldInfo.isForeign() && (DBINFO.tableByName( fieldInfo.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_complex) ) {
//            data = DisplayDataGenerator().generate( fieldInfo.m_relationDBTable, data );
//        }
//    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    /* Note: "value" must be the "id" value for any role */
    if (!item.isValid()) return false;
//    qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role =" << role << ", data =" << value.toString();
//    if (m_bNeedSave) saveData(item, role); // Spike #1

    bool editSetted = false, userSetted = false;
    int colNumb = item.column();
    if ( colNumb != 2 && (role == Qt::EditRole || role == Qt::UserRole) ) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), colNumb);
        if (fieldInf.isForeign() && (DBINFO.tableByName( fieldInf.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_complex) ) {
            QVariant generData = DisplayDataGenerator().generate(fieldInf.m_relationDBTable, value);
            editSetted = QSqlRelationalTableModel::setData(item, value, Qt::EditRole);
            userSetted = QSqlRelationalTableModel::setData(item, generData, Qt::UserRole);
//            m_generatedData[item.column()]
            qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << value << ", generData:" << generData;
        }
    }

//    if (m_bNeedSave) restoreData(item.row(), role); // Spike #1
//    qDebug() << "data after QSqlRelationalTableModel::setData:" << QSqlRelationalTableModel::data(item, role);
    qDebug() << "edit setted:" << editSetted << ", user setted:" << userSetted;
    if (editSetted && userSetted) {
        qDebug() << "emit dataChanged()";
        emit dataChanged(item, item);
    }
    return editSetted && userSetted;
}

/* Save data before calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::saveData(const QModelIndex &currentIndex, int role)
{
    qDebug() << "saveData(), role =" << role;
    const dbi::DBTInfo::T_arrDBTFieldsInfo &fieldsInf = DBINFO.tableByName(tableName())->m_fields;
    for (int col = 0; (unsigned)col < fieldsInf.size(); ++col) {
        /* if a field is not the current field and it is a foreign key to the complex DB table -> save its data */
        if ( (col != currentIndex.column()) && (fieldsInf.at(col).relationDBTtype() == dbi::DBTInfo::ttype_complex) ) {
            m_mSavedData.insert( col, QSqlRelationalTableModel::data( this->index(currentIndex.row(), col), role ) );
        }
    }
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::restoreData(int currentRow, int role)
{
    for (auto it = m_mSavedData.cbegin(); it != m_mSavedData.cend(); ++it) {
        qDebug() << "data restoring. col =" << it.key() << ", value =" << it.value().toString();
        QSqlRelationalTableModel::setData( index(currentRow, it.key()), it.value(), role );
    }
    m_mSavedData.clear();
    m_bNeedSave = false;
}

void CustomSqlTableModel::fillGeneratedData()
{
    int idValue = -1;
    for (int col = 0; (unsigned)col < columnCount(QModelIndex()); ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col);
        if ( fieldInf.relationDBTtype() == dbi::DBTInfo::ttype_complex ) {
            idValue = QSqlRelationalTableModel::data( index() )
            QVariant generData = DisplayDataGenerator().generate(fieldInf.m_relationDBTable, idValue);
            m_generatedData.insert(col, generData);
        }
    }
}

void CustomSqlTableModel::print() const
{
    qDebug() << "DB table content:";
    for (int row = 0; row < rowCount(QModelIndex()); ++row) {
        for (int col = 0; col < columnCount(QModelIndex()); ++col) {
            qDebug() << "[" << row << "," << col << "], disp:" << QSqlRelationalTableModel::data(index(row, col), Qt::DisplayRole)
                     << ", edit:" << QSqlRelationalTableModel::data(index(row, col), Qt::EditRole);
        }
        qDebug() << "---------";
    }
}

/*
 * CustomSqlRelationalDelegate
 */
CustomSqlRelationalDelegate::CustomSqlRelationalDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{ }

CustomSqlRelationalDelegate::~CustomSqlRelationalDelegate()
{ }

void CustomSqlRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    qDebug() << "delegate, row =" << index.row() << ", col =" << index.column();

    QSqlTableModel *sqlTable = qobject_cast<QSqlTableModel *>(model);
    if (!sqlTable) return;
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(sqlTable->tableName(), index.column());
    bool isForeign = fieldInf.isForeign();
    if (!isForeign)
        QItemDelegate::setModelData(editor, model, index);
    else if ( isForeign && (DBINFO.tableByName( fieldInf.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_simple) )
        setDataToSimpleDBT(editor, model, index);
}

void CustomSqlRelationalDelegate::setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    /*
     * Set data to the QSqlRelationalTableModel if the current field has relation with a simple DB table
     * (with help of the QSqlRelation class instance and used a combo box for choosing records of a simple DB table).
     * This code was taked from the QSqlRelationalDelegate::setModelData() method.
     */
    QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (sqlModel && childModel && combo) {
        int currentItem = combo->currentIndex();
        int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
        int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
                          Qt::DisplayRole);
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
                          Qt::EditRole);
    }
}
