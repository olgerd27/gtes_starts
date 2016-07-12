#ifndef DB_INFO_H
#define DB_INFO_H

#include <QString>
#include <vector>

/*
 * Database Info namespace
 */
namespace dbi {
    /*
     * Database table (DBT) field info
     */
    struct DBTFieldInfo
    {
        enum WidgetsTypes
        {
              wtype_not_show
            , wtype_label
            , wtype_lineEdit
            , wtype_comboBox
            , wtype_spinBoxInt
            , wtype_spinBoxDouble
            , wtype_plainTextEdit
        };

        bool isValid() const;
        bool isForeign() const;
        int relationDBTtype() const; /* return DBTInfo::TableTypes value. Returned value has the int type,
                                      * because the DBTInfo::TableTypes type is not known here */

        QString m_nameInDB;         /* a field name, used in database */
        QString m_nameInUI;         /* a field name, used in user interface */
        WidgetsTypes m_widgetType;  /* a type of a widget, used for showing data of this table filed */
        QString m_relationDBTable;  /* a name of another relational database table. It is like a foreign key */
    };

    /*
     * Database table (DBT) info
     */
    struct DBTInfo
    {
        struct IdentityInfo
        {
            /* An identity string is: [m_strBefore + m_NField] */
            QString m_strBefore;    /* string, before the identity field number (like a some comment) */
            int m_NField;           /* number of an identity field */
        };

        typedef std::vector<DBTFieldInfo> T_arrDBTFieldsInfo;
        typedef std::vector<IdentityInfo> T_arrIdentityFields;

        enum TableTypes
        {
              ttype_simple      // has two columns - "id" and, for example, "name". Relation with other table - "one to many"
            , ttype_complex     // has many columns. Relation with other table - "one to many"
            , ttype_composite   // has many columns. Relation with other tables - "one to many", but implement relationship between them as "many to many"
            , ttype_invalid     // invalid type of database table
        };

        enum { NO_IDENTITY_FIELD = -1 };

        int tableDegree() const;

        /*
         * Function return the found instance of the dbi::DBTFieldInfo class. If item was not found, function return empty (not valid) instance
         * of the dbi::DBTFieldInfo class and you can check returned value by calling the dbi::DBTFieldInfo::isValid() method.
         * If function argument is invalid - throw the std::invalid_argument exception.
         */
        DBTFieldInfo fieldByName(const QString &fieldName) const;

        /*
         * Function return the found instance of the dbi::DBTFieldInfo class.
         * If function argument is invalid - throw the std::out_of_range exception.
         */
        DBTFieldInfo fieldByIndex(int index) const;

        QString m_nameInDB;                 /* a table name, used in database */
        QString m_nameInUI;                 /* a table name, used in user interface */
        TableTypes m_type;                  /* a table type */
        T_arrDBTFieldsInfo m_fields;        /* array of the table fileds */
        T_arrIdentityFields m_idnFields;    /* array of the identity fields information, that use for forming the identity string */
    };

    /*
     * Database info
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

        /*
         * Function return the found point to the instance of the dbi::DBTInfo* class. If item was not found, function return 0
         * and you can check returned value. If function argument is invalid - throw the std::invalid_argument exception.
         */
        DBTInfo * tableByName(const QString &tableName) const;

    private:
        DBInfo();
        DBInfo(const DBInfo &) = delete;
        DBInfo & operator=(const DBInfo &) = delete;

        std::vector<DBTInfo *> m_tables; /* array of all DB tables */
    };

    /*
     * Calling the dbi::DBInfo::tableByName(const QString &) and the dbi::DBTInfo::fieldByName(const QString &) methods
     */
    dbi::DBTFieldInfo fieldByNames(const QString &tableName, const QString &fieldName);

    /*
     * Calling the dbi::DBInfo::tableByName(const QString &) and the dbi::DBTInfo::fieldByIndex(int) methods
     */
    dbi::DBTFieldInfo fieldByNameIndex(const QString &tableName, int fieldIndex);

    /*
     * Calling the dbi::DBTFieldInfo::isValid(), the dbi::DBTFieldInfo::isForeign() and the dbi::DBInfo::tableByName(const QString &) methods
     */
    bool isRelatedWithDBTType(const dbi::DBTFieldInfo &fieldInfo, dbi::DBTInfo::TableTypes tableType);

    /*
     * Calling the dbi::isRelatedWithDBTType(const dbi::DBTFieldInfo &, dbi::DBTInfo::TableTypes) function
     */
    bool isRelatedWithDBTType(const QString &tableName, int fieldIndex, dbi::DBTInfo::TableTypes tableType);

    /*
     * Calling the dbi::DBTFieldInfo::isValid(), the dbi::DBTFieldInfo::isForeign() and the dbi::DBInfo::tableByName(const QString &) methods.
     * If function argument is invalid or isn't a foreign key - throw the std::invalid_argument exception.
     */
    dbi::DBTInfo * relatedDBT(const DBTFieldInfo &fieldInf);
}

#define DBINFO dbi::DBInfo::Instance()

#endif // DB_INFO_H
