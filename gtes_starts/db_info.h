#ifndef DB_INFO_H
#define DB_INFO_H

#include <QString>
#include <vector>

/*
 * Field
 */
class DBTableInfo;
struct DBTableFieldInfo
{
    enum WidgetsTypes
    {
        wtype_label,
        wtype_lineEdit,
        wtype_comboBox,
        wtype_spinBoxInt,
        wtype_spinBoxDouble,
        wtype_textEdit
    };

    bool isValid() const;

    QString m_nameInDB;
    QString m_nameInUI;
    WidgetsTypes m_widgetType;
    DBTableInfo *m_ptrForeignTable;
};

/*
 * Table
 */
struct DBTableInfo
{
    enum TableTypes
    {
        ttype_simple,   // has two columns - "id" and, for example, "name". Relation with other table - "one to many"
        ttype_complex,  // has many columns. Relation with other table - "one to many"
        ttype_composite // has many columns. Relation with other tables - "one to many", but implement relationship between them as "many to many"
    };

    int tableDegree() const;
    const DBTableFieldInfo & findField(const QString &fieldName) const;

    QString m_nameInDB;
    QString m_nameInUI;
    std::vector<DBTableFieldInfo> m_fields;
};

/*
 * Database
 */
struct DBInfo
{
    static DBInfo & Instance()
    {
        static DBInfo info;
        return info;
    }

    ~DBInfo();
    DBTableInfo *findTable(const QString &tableName) const;

private:
    DBInfo();
    DBInfo(const DBInfo &) = delete;
    DBInfo & operator=(const DBInfo &) = delete;

    std::vector<DBTableInfo *> m_tables;
};
#define DBINFO DBInfo::Instance()

#endif // DB_INFO_H
