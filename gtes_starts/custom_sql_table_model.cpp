#include <QSqlQuery>
#include <QVector>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "db_info.h"

/*
 * Display Data Mode namespace
 */
//namespace ddm {
//    enum DisplayDataMode {
//        display_code,
//        display_text,
//        display_none
//    };
//    QVector<DisplayDataMode> vDisplayedModes;

//    void fillDisplayDataMode(ddm::DisplayDataMode mode, int size) { vDisplayedModes.fill(mode, size); }
//    void setDisplayDataMode(int index, ddm::DisplayDataMode mode) { vDisplayedModes[index] = mode; }

//    ddm::DisplayDataMode displayDataMode(int index)
//    {
//        return (index < 0 || index >= vDisplayedModes.size())
//                ? ddm::display_none : vDisplayedModes.at(index);
//    }

//    QString strDisplayDataMode(DisplayDataMode mode)
//    {
//        QString str;
//        switch (mode) {
//        case display_code:
//            str = "Display code";
//            break;
//        case display_text:
//            str = "Display text";
//            break;
//        case display_none:
//            str = "Display none";
//            break;
//        default:
//            str = "Display unknow";
//            break;
//        }
//        return str;
//    }
//}

/*
 * CustomSqlTableModel
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
{
}

void CustomSqlTableModel::setTable(const QString &table)
{
//    ddm::fillDisplayDataMode(ddm::display_text, size);
    QSqlRelationalTableModel::setTable(table);
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    QVariant data = QSqlRelationalTableModel::data(item, role);
    int colNumb = item.column();

//    qDebug() << "-data(): col =" << colNumb << ", role =" << role
//             << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(colNumb))
//             << ", data =" << data.toString();

//    if ( ddm::vDisplayedModes.at(colNumb) == ddm::display_text ) {
//        qDebug() << "+data(): col =" << colNumb << ", role =" << role
//                 << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(colNumb))
//                 << ", data =" << data.toString();
//        if (role == Qt::DisplayRole)
//            qDebug() << "display role: col =" << colNumb
//                     << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(colNumb))
//                     << ", data =" << data.toString();
//        if ( colNumb == 1 && (role == Qt::EditRole || role == Qt::DisplayRole) ) {
        if ( colNumb == 1 && role == Qt::EditRole ) {
//            qDebug() << "<data(): col =" << colNumb << ", role =" << role
//                     << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(colNumb))
//                     << ", data =" << data.toString();
            dbi::DBTFieldInfo fieldInfo = dbi::fieldByNameIndex(tableName(), colNumb);
            if (fieldInfo.isForeign())
                data = getDisplayData( fieldInfo.m_relationDBTable, data.toInt() );
//            exit(1);
        }
//    }
//    else
//        ddm::setDisplayDataMode(colNumb, ddm::display_text);
//    qDebug() << "!data(): col =" << colNumb << ", role =" << role
//             << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(colNumb))
//             << ", data =" << data.toString();
    return data;
}

QString CustomSqlTableModel::getDisplayData(const QString &tableName, QVariant varId) const
{
    qDebug() << "getDisplayData()";
    /* get the "id" integer value */
    bool b = false;
    int id = varId.toInt(&b);
    if (!b) {
        // TODO: generate error message
        qDebug() << "Error! Title: Conversion error. Message: Cannot convert the foreign key value \""
                 << varId << "\" from the variant type to the integer type";
        return varId.toString();
    }

    /******************************** Queries autogeneration **************************************/
//    QString strQuery("SELECT ");
    QString strRes("");
    int counter = 0;
    strRes = relationalDBTdata(DBINFO.tableByName(tableName), strRes, counter);
//    qDebug() << "with placeholders:" << strRes;

    /* Retrieve appropriate data for generating identification strings */
    QString strQuery1 =
            QString("SELECT "
                    "names_engines.name, names_modifications_engines.modification, full_names_engines.number "
                    "FROM "
                    "names_engines, names_modifications_engines, full_names_engines "
                    "WHERE "
                    "full_names_engines.id = %1 "
                    "AND full_names_engines.name_modif_id = names_modifications_engines.id "
                    "AND names_modifications_engines.name_id = names_engines.id;").arg(id);
    QSqlQuery query(strQuery1);
    if (query.next()) {
        for (int field = 0; field < counter; field++)
            strRes = strRes.arg( query.value(field).toString() );
    }
    else {
        // TODO: generate error message
        qDebug() << tr("Title: Error data obtaining."
                       "Message: Cannot get a data from the database for generating displayed data of the \"%1\" database table.\n"
                       "The reason is: invalid database query.\n\n"
                       "Please consult with the application developer for fixing this problem.").arg(tableName);
        strRes.clear();
    }
//    qDebug() << "result:" << strRes;
    return strRes;
}

QString & CustomSqlTableModel::relationalDBTdata(dbi::DBTInfo *table, QString &data, int &fieldCounter) const
{
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        const dbi::DBTFieldInfo &field = table->fieldByIndex( idnField.m_NField );
        data += idnField.m_strBefore;
        data = field.isForeign()
               ? relationalDBTdata(DBINFO.tableByName(field.m_relationDBTable), data, fieldCounter) /* recursive calling */
               : data + QString("%%1").arg(++fieldCounter); /* when current field isn't a foreign key -> exit from recursion */
    }
    return data;
}

QString CustomSqlTableModel::generateQuery(dbi::DBTInfo *table) const
{
    QString query("SELECT ");
    table;
    return query;
}


bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    if (!item.isValid()) return false;
//    static int cc = 0;
//    int col = item.column();
//    qDebug() << "setData(" << cc << ") start: col =" << col << ", role =" << role
//             << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(col))
//             << ", data =" << value.toString();
//    ddm::setDisplayDataMode(col, ddm::display_code);
//    qDebug() << ">>>BEFORE, mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(col));
    QSqlRelationalTableModel::setData(item, value, role);
//    qDebug() << "<<<AFTER, mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(col));
//    ddm::setDisplayDataMode(col, ddm::display_text);
//    qDebug() << "setData(" << cc << ") end: col =" << col << ", role =" << role
//             << ", mode =" << ddm::strDisplayDataMode(ddm::displayDataMode(col))
//             << ", data =" << value.toString();
    emit dataChanged(item, item);
//    ++cc;
//    qDebug() << "===========================================================";
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
