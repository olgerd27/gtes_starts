#include <QObject>
#include <QDebug>
#include <stdexcept>
#include <algorithm>
#include "db_info.h"

/*
 * DBTFieldInfo
 */
bool dbi::DBTFieldInfo::isValid() const
{
    return !m_nameInDB.isEmpty() && !m_nameInUI.isEmpty();
}

bool dbi::DBTFieldInfo::isForeign() const
{
    return isValid() && !m_relationDBTable.isEmpty();
}

int dbi::DBTFieldInfo::relationDBTtype() const
{
    return isForeign() ? DBINFO.tableByName(m_relationDBTable)->m_type : DBTInfo::ttype_simple;
}

/*
 * Comparison a some database item (table, field and so on (the name, used in DB, passed in QString format))
 * with the template some "info" class instance.
 * A some template "info" class instance must have variable "m_nameInDB".
 * Have 2 overloaded version of functor predicate with pointer and refference arguments.
 */
template<typename T_info>
struct CompareInfoWithString
{
    CompareInfoWithString(const QString &itemNameInDB)
    {
        m_info.m_nameInDB = itemNameInDB;
    }

    inline bool operator()(const T_info &item)
    {
        return operator ()(&item);
    }

    inline bool operator()(const T_info *item)
    {
        return item->m_nameInDB == m_info.m_nameInDB;
    }

private:
    T_info m_info;
};

/*
 * DBTInfo
 */
int dbi::DBTInfo::tableDegree() const
{
    return m_fields.size();
}

dbi::DBTFieldInfo dbi::DBTInfo::fieldByName(const QString &fieldName) const
{
    if (fieldName.isEmpty())
        throw std::invalid_argument("Cannot return field by name - the string argument is empty. "
                                    "Please set a valid data");
    auto it = std::find_if(m_fields.begin(), m_fields.end(), CompareInfoWithString<DBTFieldInfo>(fieldName));
    return it == m_fields.end() ? DBTFieldInfo() : *it;
}

dbi::DBTFieldInfo dbi::DBTInfo::fieldByIndex(int index) const
{
    if (index < 0 || index >= (int)m_fields.size())
        throw std::out_of_range( QString("Cannot return the dbi::DBTFieldInfo instance, the argument index \"%1\" is out of range")
                                 .arg(index).toStdString() );
    return m_fields[index];
}

/*
 * DBInfo
 */
QString trans(const char *s, const char *c = 0, int n = -1)
{
    return QObject::tr(s, c, n);
}

dbi::DBInfo::DBInfo()
{
    m_tables = {
        /* names_engines */
        new DBTInfo {
            "names_engines", trans("Engine name"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            {
                {"", 1}
            }
        },

        /* names_modifications_engines */
        new DBTInfo {
            "names_modifications_engines", trans("Engine name and modification"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name_id", trans("Name"), DBTFieldInfo::wtype_lineEdit, "names_engines"}
                , {"modification", trans("Modification"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            {
                  {"", 1}, {" ", 2}
            }
        },

        /* full_names_engines */
        new DBTInfo {
            "full_names_engines", trans("Full name"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name_modif_id", trans("Name and modification"), DBTFieldInfo::wtype_lineEdit, "names_modifications_engines"}
                , {"number", trans("Number"), DBTFieldInfo::wtype_spinBoxInt, ""}
            },
            {
                  {"", 1}, {QString(" %1").arg(trans("#")), 2}
            }
        },

        /* fuels_types */
        new DBTInfo {
            "fuels_types", trans("Fuel type"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            {
                {"", 1}
            }
        },

        /* start_devices_types */
        new DBTInfo {
            "start_devices_types", trans("Start device type"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            {
                {"", 1}
            }
        },

        /* start_devices */
        new DBTInfo {
            "start_devices", trans("Start device"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"device_type_id", trans("Type"), DBTFieldInfo::wtype_lineEdit, "start_devices_types"}
                , {"model", trans("Model"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"Nnom", trans("Nnom"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"n_nom", trans("n_nom"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"kp", trans("kp"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"f1", trans("f1"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"f2", trans("f2"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
            },
            {
                  {"", 1}, {" ", 2}
            }
        },

        /* injectors_types */
        new DBTInfo {
            "injectors_types", trans("Injector types"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            {
                {"", 1}
            }
        },

        /* combustion_chambers */
        new DBTInfo {
            "combustion_chambers", trans("Combustion chamber"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"draft_number", trans("Draft number"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"flue_tubes_quantity", trans("Flue tubes quantity"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"injectors_type_id", trans("Injector type"), DBTFieldInfo::wtype_lineEdit, "injectors_types"}
                , {"igniters_quantity", trans("Igniters quantity"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
            },
            {
                  {"#", 0}
                , {QString(", %1: ").arg(trans("draft")), 1}
                /* , {QString(", %1 = ").arg(trans("FT", "This is a Flame Tube")), 2} */
            }
        },

        /* engines */
        new DBTInfo {
            "engines", trans("Engine"), DBTInfo::ttype_composite,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"full_name_id", trans("Identification"), DBTFieldInfo::wtype_lineEdit, "full_names_engines"}
                , {"fuel_type_id", trans("Fuel type"), DBTFieldInfo::wtype_lineEdit, "fuels_types"}
                , {"combustion_chamber_id", trans("Combustion chamber"), DBTFieldInfo::wtype_lineEdit, "combustion_chambers"}
                , {"start_device_id", trans("Start device"), DBTFieldInfo::wtype_lineEdit, "start_devices"}
                , {"start_devices_quantity", trans("Start device quantity"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
            },
            {
                  {"#", 0}
                , {", ", 1}
                , {QString(", %1: ").arg(trans("fuel")), 2}
            }
        }
    };
}

struct DeletePtrData
{
    template<typename T> void operator()(T *ptr) { delete ptr; }
};

dbi::DBInfo::~DBInfo()
{
    std::for_each(m_tables.begin(), m_tables.end(), DeletePtrData());
}

QString dbi::DBInfo::name() const
{
    return "gtes_starts";
}

dbi::DBTInfo * dbi::DBInfo::tableByName(const QString &tableName) const
{
    if (tableName.isEmpty())
        throw std::invalid_argument("The string argument to dbi::DBInfo::tableByName(QString) function is empty. "
                                    "Please set a valid data");
    auto it = std::find_if(m_tables.begin(), m_tables.end(), CompareInfoWithString<DBTInfo>(tableName));
    return it == m_tables.end() ? 0 : *it;
}

/*
 * External convenient function
 */
dbi::DBTFieldInfo dbi::fieldByNames(const QString &tableName, const QString &fieldName)
{
    DBTInfo *tblInf = DBINFO.tableByName(tableName);
    return tblInf ? tblInf->fieldByName(fieldName) : DBTFieldInfo();
}

dbi::DBTFieldInfo dbi::fieldByNameIndex(const QString &tableName, int fieldIndex)
{
    DBTInfo *tblInf = DBINFO.tableByName(tableName);
    return tblInf ? tblInf->fieldByIndex(fieldIndex) : DBTFieldInfo();
}

bool dbi::isRelatedWithDBTType(const dbi::DBTFieldInfo &fieldInfo, dbi::DBTInfo::TableTypes tableType)
{
    return fieldInfo.isForeign() && (DBINFO.tableByName( fieldInfo.m_relationDBTable )->m_type == tableType);
}

bool dbi::isRelatedWithDBTType(const QString &tableName, int fieldIndex, DBTInfo::TableTypes tableType)
{
    return isRelatedWithDBTType(fieldByNameIndex(tableName, fieldIndex), tableType);
}

dbi::DBTInfo *dbi::relatedDBT(const dbi::DBTFieldInfo &fieldInf)
{
    if (!fieldInf.isForeign())
        throw std::invalid_argument("Cannot get the info of the related database table (DBTInfo class instance). "
                                    "The argument of the dbi::relatedDBT(dbi::DBTFieldInfo) method is not valid -> field is not foreign");
    return DBINFO.tableByName(fieldInf.m_relationDBTable);
}
