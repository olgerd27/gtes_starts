#include <QObject>
#include <QDebug>
#include "db_info.h"

/*
 * DBTableFieldInfo
 */
bool DBTableFieldInfo::isValid() const
{
    return !m_nameInDB.isEmpty() && !m_nameInUI.isEmpty();
}

/*
 * Comparison a some database table (the name passed in QString format) with the template some "info" class instance.
 * A some template "info" class instance must have variable "m_nameInDB".
 * Have 2 overloaded version of functor predicate with pointer and refference arguments.
 */
template<typename T_info>
struct CompareInfoWithString
{
    CompareInfoWithString(const QString &tableName)
    {
        m_info.m_nameInDB = tableName;
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
 * DBTableInfo
 */
int DBTableInfo::tableDegree() const
{
    return m_fields.size();
}

DBTableFieldInfo DBTableInfo::fieldByName(const QString &fieldName) const
{
    auto it = std::find_if(m_fields.begin(), m_fields.end(), CompareInfoWithString<DBTableFieldInfo>(fieldName));
    return it == m_fields.end() ? DBTableFieldInfo() : *it;
}

DBTableFieldInfo DBTableInfo::fieldByIndex(int index) const
{
    return (index < 0 || index >= (int)m_fields.size()) ? DBTableFieldInfo() : m_fields.at(index);
}

/*
 * DBInfo
 */
DBInfo::DBInfo()
{
    m_tables = {
        /* names_engines */
        new DBTableInfo {
            "names_engines", QObject::tr("Engine name"), DBTableInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* names_modifications_engines */
        new DBTableInfo {
            "names_modifications_engines", QObject::tr("Engine name and modification"), DBTableInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name_id", QObject::tr("Name"), DBTableFieldInfo::wtype_comboBox, "names_engines"}
                , {"modification", QObject::tr("Modification"), DBTableFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1}, {" ", 2} }
        },

        /* identification_data_engines */
        new DBTableInfo {
            "identification_data_engines", QObject::tr("Engines identification data"), DBTableInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name_modif_id", QObject::tr("Name and modification"), DBTableFieldInfo::wtype_comboBox, "names_modifications_engines"}
                , {"number", QObject::tr("Number"), DBTableFieldInfo::wtype_spinBoxInt, ""}
            },
            { {"#", 0}, {", ", 1}, {QString(" %1").arg(QObject::tr("#")), 2} }
        },

        /* fuels_types */
        new DBTableInfo {
            "fuels_types", QObject::tr("Fuel type"), DBTableInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* start_devices_types */
        new DBTableInfo {
            "start_devices_types", QObject::tr("Start device type"), DBTableInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* start_devices */
        new DBTableInfo {
            "start_devices", QObject::tr("Start device"), DBTableInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"device_type_id", QObject::tr("Type"), DBTableFieldInfo::wtype_comboBox, "start_devices_types"}
                , {"model", QObject::tr("Model"), DBTableFieldInfo::wtype_lineEdit, ""}
                , {"Nnom", QObject::tr("Nnom"), DBTableFieldInfo::wtype_spinBoxDouble, ""}
                , {"n_nom", QObject::tr("n_nom"), DBTableFieldInfo::wtype_spinBoxDouble, ""}
                , {"kp", QObject::tr("kp"), DBTableFieldInfo::wtype_spinBoxDouble, ""}
                , {"f1", QObject::tr("f1"), DBTableFieldInfo::wtype_spinBoxDouble, ""}
                , {"f2", QObject::tr("f2"), DBTableFieldInfo::wtype_spinBoxDouble, ""}
                , {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, ""}
            },
            { {"#", 0}, {", ", 2} }
        },

        /* injectors_types */
        new DBTableInfo {
            "injectors_types", QObject::tr("Injector types"), DBTableInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* combustion_chambers */
        new DBTableInfo {
            "combustion_chambers", QObject::tr("Combustion chamber"), DBTableInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"draft_number", QObject::tr("Draft number"), DBTableFieldInfo::wtype_lineEdit, ""}
                , {"flue_tubes_quantity", QObject::tr("Flue tubes quantity"), DBTableFieldInfo::wtype_lineEdit, ""}
                , {"injectors_type_id", QObject::tr("Injector type"), DBTableFieldInfo::wtype_comboBox, "injectors_types"}
                , {"igniters_quantity", QObject::tr("Igniters quantity"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* engines */
        new DBTableInfo {
            "engines", QObject::tr("Engine"), DBTableInfo::ttype_composite,
            {
                  {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"identification_id", QObject::tr("Identification"), DBTableFieldInfo::wtype_label, "identification_data_engines"}
                , {"fuel_type_id", QObject::tr("Fuel type"), DBTableFieldInfo::wtype_comboBox, "fuels_types"}
                , {"combustion_chamber_id", QObject::tr("Combustion chamber"), DBTableFieldInfo::wtype_label, "combustion_chambers"}
                , {"start_device_id", QObject::tr("Start device"), DBTableFieldInfo::wtype_label, "start_devices"}
                , {"start_devices_quantity", QObject::tr("Start device quantity"), DBTableFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, ""}
            },
            { {"#", 0}, {", ", 1}, {QString(", %1: ").arg(QObject::tr("fuel")), 2} }
        }
    };
}

struct DeletePtrData
{
    template<typename T> void operator()(T *ptr) { delete ptr; }
};

DBInfo::~DBInfo()
{

}

QString DBInfo::name() const
{
    return "gtes_starts";
}

DBTableInfo * DBInfo::findTable(const QString &tableName) const
{
    auto it = std::find_if(m_tables.begin(), m_tables.end(), CompareInfoWithString<DBTableInfo>(tableName));
    return it == m_tables.end() ? 0 : *it;
}
