#ifndef DB_INFO_H
#define DB_INFO_H

#include <QString>
#include <vector>

/*
 * Field
 */
struct DBTableFieldInfo
{
    enum WidgetsTypes
    {
          wtype_not_show
        , wtype_label
        , wtype_lineEdit
        , wtype_comboBox
        , wtype_spinBoxInt
        , wtype_spinBoxDouble
        , wtype_textEdit
    };

    bool isValid() const;

    QString m_nameInDB;         /* a field name, used in database */
    QString m_nameInUI;         /* a field name, used in user interface */
    WidgetsTypes m_widgetType;  /* a type of a widget, used for showing data of this table filed */
    QString m_relationDBTable;  /* a name of another relational database table. It is like a foreign key */
};

/*
 * Table
 */
struct DBTableInfo
{
    struct IdentityInfo
    {
        /* An identity string is: [m_strName + m_NField] */
        QString m_strName;  /* comment to the identity field number */
        int m_NField;       /* number of an identity field */
    };

    typedef std::vector<DBTableFieldInfo> T_arrTableInfos;
    typedef std::vector<IdentityInfo> T_arrIdentityFields;

    enum TableTypes
    {
          ttype_simple      // has two columns - "id" and, for example, "name". Relation with other table - "one to many"
        , ttype_complex     // has many columns. Relation with other table - "one to many"
        , ttype_composite   // has many columns. Relation with other tables - "one to many", but implement relationship between them as "many to many"
    };

    enum { NO_IDENTITY_FIELD = -1 };

    int tableDegree() const;
    DBTableFieldInfo fieldByName(const QString &fieldName) const;
    DBTableFieldInfo fieldByIndex(int index) const;

    QString m_nameInDB;                 /* a table name, used in database */
    QString m_nameInUI;                 /* a table name, used in user interface */
    TableTypes m_type;                  /* a table type */
    T_arrTableInfos m_fields;           /* array of the table fileds */
    T_arrIdentityFields m_idnFields;    /* array of the identity fields information, that use for forming the identity string */
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
    QString name() const;
    DBTableInfo *findTable(const QString &tableName) const;

private:
    DBInfo();
    DBInfo(const DBInfo &) = delete;
    DBInfo & operator=(const DBInfo &) = delete;

    std::vector<DBTableInfo *> m_tables; /* array of all DB tables */
};
#define DBINFO DBInfo::Instance()

#endif // DB_INFO_H
