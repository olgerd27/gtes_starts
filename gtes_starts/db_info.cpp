#include <QObject>
#include <QDebug>
#include "db_info.h"

/*
 * DBTFieldInfo
 */
bool DBTFieldInfo::isValid() const
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
 * DBTInfo
 */
int DBTInfo::tableDegree() const
{
    return m_fields.size();
}

DBTFieldInfo DBTInfo::fieldByName(const QString &fieldName) const
{
    auto it = std::find_if(m_fields.begin(), m_fields.end(), CompareInfoWithString<DBTFieldInfo>(fieldName));
    return it == m_fields.end() ? DBTFieldInfo() : *it;
}

DBTFieldInfo DBTInfo::fieldByIndex(int index) const
{
    return (index < 0 || index >= (int)m_fields.size()) ? DBTFieldInfo() : m_fields.at(index);
}

/*
 * DBInfo
 */
DBInfo::DBInfo()
{
    m_tables = {
        /* names_engines */
        new DBTInfo {
            "names_engines", QObject::tr("Engine name"), DBTInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* names_modifications_engines */
        new DBTInfo {
            "names_modifications_engines", QObject::tr("Engine name and modification"), DBTInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name_id", QObject::tr("Name"), DBTFieldInfo::wtype_comboBox, "names_engines"}
                , {"modification", QObject::tr("Modification"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1}, {" ", 2} }
        },

        /* identification_data_engines */
        new DBTInfo {
            "identification_data_engines", QObject::tr("Engines identification data"), DBTInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name_modif_id", QObject::tr("Name and modification"), DBTFieldInfo::wtype_comboBox, "names_modifications_engines"}
                , {"number", QObject::tr("Number"), DBTFieldInfo::wtype_spinBoxInt, ""}
            },
            { {"#", 0}, {", ", 1}, {QString(" %1").arg(QObject::tr("#")), 2} }
        },

        /* fuels_types */
        new DBTInfo {
            "fuels_types", QObject::tr("Fuel type"), DBTInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* start_devices_types */
        new DBTInfo {
            "start_devices_types", QObject::tr("Start device type"), DBTInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* start_devices */
        new DBTInfo {
            "start_devices", QObject::tr("Start device"), DBTInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"device_type_id", QObject::tr("Type"), DBTFieldInfo::wtype_comboBox, "start_devices_types"}
                , {"model", QObject::tr("Model"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"Nnom", QObject::tr("Nnom"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"n_nom", QObject::tr("n_nom"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"kp", QObject::tr("kp"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"f1", QObject::tr("f1"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"f2", QObject::tr("f2"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"comments", QObject::tr("Comments"), DBTFieldInfo::wtype_textEdit, ""}
            },
            { {"#", 0}, {", ", 2} }
        },

        /* injectors_types */
        new DBTInfo {
            "injectors_types", QObject::tr("Injector types"), DBTInfo::ttype_simple,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"name", QObject::tr("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* combustion_chambers */
        new DBTInfo {
            "combustion_chambers", QObject::tr("Combustion chamber"), DBTInfo::ttype_complex,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"draft_number", QObject::tr("Draft number"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"flue_tubes_quantity", QObject::tr("Flue tubes quantity"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"injectors_type_id", QObject::tr("Injector type"), DBTFieldInfo::wtype_comboBox, "injectors_types"}
                , {"igniters_quantity", QObject::tr("Igniters quantity"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", QObject::tr("Comments"), DBTFieldInfo::wtype_textEdit, ""}
            },
            { {"#", 0}, {", ", 1} }
        },

        /* engines */
        new DBTInfo {
            "engines", QObject::tr("Engine"), DBTInfo::ttype_composite,
            {
                  {"id", QObject::tr("Id"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"identification_id", QObject::tr("Identification"), DBTFieldInfo::wtype_label, "identification_data_engines"}
                , {"fuel_type_id", QObject::tr("Fuel type"), DBTFieldInfo::wtype_comboBox, "fuels_types"}
                , {"combustion_chamber_id", QObject::tr("Combustion chamber"), DBTFieldInfo::wtype_label, "combustion_chambers"}
                , {"start_device_id", QObject::tr("Start device"), DBTFieldInfo::wtype_label, "start_devices"}
                , {"start_devices_quantity", QObject::tr("Start device quantity"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", QObject::tr("Comments"), DBTFieldInfo::wtype_textEdit, ""}
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

DBTInfo * DBInfo::findTable(const QString &tableName) const
{
    auto it = std::find_if(m_tables.begin(), m_tables.end(), CompareInfoWithString<DBTInfo>(tableName));
    return it == m_tables.end() ? 0 : *it;
}
