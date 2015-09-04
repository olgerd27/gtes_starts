#include <QObject>
#include "db_info.h"

/*
 * DBInfo
 */
DBInfo::DBInfo()
{
//    DBTableFieldInfo fieldInfo {QObject::tr("Id"), DBTableFieldInfo::wtype_lineEdit, 0};
//    {QObject::tr("Id"), DBTableFieldInfo::wtype_lineEdit, 0};

    m_tables = {
        /* engines_names */
        new DBTableInfo {"engines_names", QObject::tr("engine name"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* fuels_types */
        new DBTableInfo {"fuels_types", QObject::tr("fuel type"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* start_devices_types */
        new DBTableInfo {"start_devices_types", QObject::tr("start device type"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* start_devices */
        new DBTableInfo {"start_devices", QObject::tr("start device"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {QObject::tr("model"), DBTableFieldInfo::wtype_lineEdit, 0},
                             {QObject::tr("Nnom"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {QObject::tr("n_nom"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {QObject::tr("kp"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {QObject::tr("f1"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {QObject::tr("f2"), DBTableFieldInfo::wtype_spinBoxDouble, 0},
                             {QObject::tr("comments"), DBTableFieldInfo::wtype_textEdit, 0}
                         }
        },

        /* injectors_types */
        new DBTableInfo {"injectors_types", QObject::tr("injector types"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("name"), DBTableFieldInfo::wtype_lineEdit, 0}
                         }
        },

        /* combustion_chambers */
        new DBTableInfo {"combustion_chambers", QObject::tr("combustion chamber"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("draft number"), DBTableFieldInfo::wtype_lineEdit, 0},
                             {QObject::tr("injector type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {QObject::tr("igniters quantity"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("comments"), DBTableFieldInfo::wtype_textEdit, 0}
                         }
        },

        /* engines */
        new DBTableInfo {"engines", QObject::tr("engine"),
                         {
                             {QObject::tr("id"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("name"), DBTableFieldInfo::wtype_comboBox, 0},
                             {QObject::tr("number"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("fuel type"), DBTableFieldInfo::wtype_comboBox, 0},
                             {QObject::tr("combustion chamber"), DBTableFieldInfo::wtype_label, 0},
                             {QObject::tr("start device"), DBTableFieldInfo::wtype_label, 0},
                             {QObject::tr("start device quantity"), DBTableFieldInfo::wtype_spinBoxInt, 0},
                             {QObject::tr("comments"), DBTableFieldInfo::wtype_textEdit, 0}
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

/* Comparison a some database table (the name passed in QString format) with pointer to the DBTableInfo class instance */
struct CompareDBTableInfoWithString
{
    CompareDBTableInfoWithString(const QString &tableName)
    {
        m_DBTableInfo.m_nameInDB = tableName;
    }

    inline bool operator()(const DBTableInfo *item)
    {
        return item->m_nameInDB == m_DBTableInfo.m_nameInDB;
    }

private:
    DBTableInfo m_DBTableInfo;
};

DBTableInfo * DBInfo::findTable(const QString &tableName) const
{
    auto it = std::find_if(m_tables.begin(), m_tables.end(), CompareDBTableInfoWithString(tableName));
    return it == m_tables.end() ? 0 : *it;
}
