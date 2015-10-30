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
        qDebug() << "mask:" << strRes;
        generateResultData( safeIdToInt(varId), strRes, fieldsCounter, tableName );
    }
    catch (const cmmn::MessageException &me) {
        if (me.type() == cmmn::MessageException::type_critical)
            strRes.clear();
        qDebug() << "[ERROR]:" << me.title() << "\n" << me.message();
//        QString("\n%1: [%2]").arg(tr("Error placement")).arg(me.placement()); // TODO: add this info to main message and pass all to the message box
        return strRes;
    }
    catch (const std::exception &ex) {
        strRes.clear();
        return strRes;
    }

    qDebug() << "result:" << strRes;
    return strRes;
}

int DisplayDataGenerator::safeIdToInt(const QVariant &value) const
{
    bool b = false;
    int id = value.toInt(&b);
    if (!b)
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the foreign key value \"%1\" from the QVariant to the integer type")
                                      .arg(value.toString()), "DisplayDataGenerator::safeIdToInt");
    return id;
}

/* Generate data mask and data for generation query string */
void DisplayDataGenerator::generate_Mask_QueryData(dbi::DBTInfo *table, QString &data, int &fieldCounter)
{
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        const dbi::DBTFieldInfo &field = table->fieldByIndex( idnField.m_NField );
        data += idnField.m_strBefore;
        if (field.isForeign()) {
//            qDebug() << "[" << fieldCounter << "] WHERE:" << table->m_nameInDB << "." << field.m_nameInDB;
            m_queryGen.addWhere( table->m_nameInDB + ".id" );
            m_queryGen.addWhere( table->m_nameInDB + "." + field.m_nameInDB );
            generate_Mask_QueryData( DBINFO.tableByName(field.m_relationDBTable), data, fieldCounter ); /* recursive calling */
        }
        else {
            data += QString("%%1").arg(++fieldCounter); /* when current field isn't a foreign key -> exit from recursion */
//            qDebug() << "[" << fieldCounter << "] SELECT:" << table->m_nameInDB << "." << field.m_nameInDB;
//            qDebug() << "[" << fieldCounter << "] FROM:" << table->m_nameInDB;
            m_queryGen.addSelect( table->m_nameInDB + "." + field.m_nameInDB );
            m_queryGen.addFrom( table->m_nameInDB );
        }
    }
}

/* Run query for retriving data from relational DB tables and generate result string data for displaying */
void DisplayDataGenerator::generateResultData(int id, QString &strRes, int fieldsNumber, const QString &tableName)
{
    QString strQuery = m_queryGen.generateQuery(id);
    qDebug() << "query:" << strQuery;
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
//    if (m_listSelect.size() != m_listFrom.size())
//        throw cmmn::MessageException( cmmn::MessageException::type_critical,
//                                      QObject::tr("Query generation error"),
//                                      QObject::tr("Error with result query generation.\n"
//                                                  "The sizes of the \"SELECT\" list and the \"FROM\" list are not equal)"),
//                                      "DisplayDataGenerator::QueryGenerator::generateQuery" );
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
    strWhere += ";";
    strWhere = strWhere.arg(id);

    qDebug() << "SELECT:" << m_listSelect.join(", ");
    qDebug() << "FROM:" << m_listFrom.join(", ");
    qDebug() << "WHERE:" << strWhere;

    QString strQuery = QString("SELECT %1 FROM %2 WHERE %3").arg( m_listSelect.join(", ") ).arg( m_listFrom.join(", ") ).arg( strWhere );
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
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
{ }

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setRelation(2, QSqlRelation("fuels_types", "id", "name"));
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    QVariant data = QSqlRelationalTableModel::data(item, role);
    int colNumb = item.column();
        if ( /*colNumb == 2 &&*/ role == Qt::EditRole ) {
            dbi::DBTFieldInfo fieldInfo = dbi::fieldByNameIndex(tableName(), colNumb);
            if (fieldInfo.isForeign()) {
                data = DisplayDataGenerator().generate( fieldInfo.m_relationDBTable, data );
            }
        }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    if (!item.isValid()) return false;
    QSqlRelationalTableModel::setData(item, value, role);
    emit dataChanged(item, item);
    return true;
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

    QSqlTableModel *sqlTable = qobject_cast<QSqlTableModel *>(model);
    if (!sqlTable) return;
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(sqlTable->tableName(), index.column());
    if (!fieldInf.isForeign())
        QItemDelegate::setModelData(editor, model, index);

//        QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
//        QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
//        QComboBox *combo = qobject_cast<QComboBox *>(editor);
//        if (!sqlModel || !childModel || !combo) {
//            qDebug() << "Before QItemDelegate::setModelData(), sqlModel:" << (void *)sqlModel
//                     << ", childModel:" << (void *)childModel << ", combo:" << (void *)combo;
//            QItemDelegate::setModelData(editor, model, index);
//            return;
//        }
//        qDebug() << "Skip QItemDelegate::setModelData()";

//        int currentItem = combo->currentIndex();
//        int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
//        int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
//        sqlModel->setData(index,
//                childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
//                Qt::DisplayRole);
//        sqlModel->setData(index,
//                childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
//                Qt::EditRole);
}
