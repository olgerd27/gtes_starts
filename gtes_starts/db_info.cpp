#include <QObject>
#include "db_info.h"

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
 * DBTableFieldInfo
 */
bool DBTableFieldInfo::isValid() const
{
    return !m_nameInDB.isEmpty() && !m_nameInUI.isEmpty();
}

/*
 * DBTableInfo
 */
int DBTableInfo::tableDegree() const
{
    return m_fields.size();
}

const DBTableFieldInfo & DBTableInfo::findField(const QString &fieldName) const
{
    auto it = std::find_if(m_fields.begin(), m_fields.end(), CompareInfoWithString<DBTableFieldInfo>(fieldName));
    return it == m_fields.end() ? DBTableFieldInfo() : *it;
}

/*
 * DBInfo
 */
DBInfo::DBInfo()
{
//    DBTableFieldInfo fieldInfo {QObject::tr("Id"), DBTableFieldInfo::wtype_lineEdit, 0};
//    {QObject::tr("Id"), DBTableFieldInfo::wtype_lineEdit, 0};

    m_tables = {
        /* engines_names */
        new DBTableInfo {"engines_names", QObject::tr("Engine name"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* fuels_types */
        new DBTableInfo {"fuels_types", QObject::tr("Fuel type"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* start_devices_types */
        new DBTableInfo {"start_devices_types", QObject::tr("Start device type"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* start_devices */
        new DBTableInfo {"start_devices", QObject::tr("Start device"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"type", QObject::tr("Type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {"model", QObject::tr("Model"), DBTableFieldInfo::wtype_lineEdit, 0},
                             {"Nnom", QObject::tr("Nnom"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {"n_nom", QObject::tr("n_nom"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {"kp", QObject::tr("kp"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {"f1", QObject::tr("f1"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {"f2", QObject::tr("f2"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, 0}
                         }
        },

        /* injectors_types */
        new DBTableInfo {"injectors_types", QObject::tr("Injector types"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* combustion_chambers */
        new DBTableInfo {"combustion_chambers", QObject::tr("Combustion chamber"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"draft number", QObject::tr("Draft number"), DBTableFieldInfo::wtype_lineEdit, 0},
                             {"injector type", QObject::tr("Injector type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {"igniters quantity", QObject::tr("Igniters quantity"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, 0}
                         }
        },

        /* engines */
        new DBTableInfo {"engines", QObject::tr("Engine"),
                         {
                             {"id", QObject::tr("Id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"name", QObject::tr("Name"), DBTableFieldInfo::wtype_comboBox, 0},
                             {"number", QObject::tr("Number"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"fuel type", QObject::tr("Fuel type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {"combustion chamber", QObject::tr("Combustion chamber"), DBTableFieldInfo::wtype_label, 0},
                             {"start device", QObject::tr("Start device"), DBTableFieldInfo::wtype_label, 0},
                             {"start device quantity", QObject::tr("Start device quantity"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {"comments", QObject::tr("Comments"), DBTableFieldInfo::wtype_textEdit, 0}
                         }
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

DBTableInfo * DBInfo::findTable(const QString &tableName) const
{
    auto it = std::find_if(m_tables.begin(), m_tables.end(), CompareInfoWithString<DBTableInfo>(tableName));
    return it == m_tables.end() ? 0 : *it;
}
