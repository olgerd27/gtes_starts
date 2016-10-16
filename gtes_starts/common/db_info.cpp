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

bool dbi::DBTFieldInfo::isPrimary() const
{
    return m_nameInDB == "id";
}

bool dbi::DBTFieldInfo::isForeign() const
{
    return isValid() && !m_relationDBTable.isEmpty();
}

bool dbi::DBTFieldInfo::isKey() const
{
    return isPrimary() || isForeign();
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
    /*
     * TODO: there are need to move this data to an external file, like XML.
     * Also, it is need to create the specific xml editor for altering structure of DB outside of this app.
     * This is possible only if app is entirely flexible, i.e. can adapt to any DB structure.
     */
    m_tables = {
        /* names_engines */
        new DBTInfo {
            "names_engines", trans("Engines names"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      DG90
            {
                  {"", 1}
            }
        },

        /* names_modifications_engines */
        new DBTInfo {
            "names_modifications_engines", trans("Engines names and modifications"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name_id", trans("Name"), DBTFieldInfo::wtype_lineEdit, "names_engines"}
                , {"modification", trans("Modification"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      DG90 L2.1
            {
                  {"", 1}
                , {" ", 2}
            }
        },

        /* full_names_engines */
        new DBTInfo {
            "full_names_engines", trans("Full engines names"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name_modif_id", trans("Name and modification"), DBTFieldInfo::wtype_lineEdit, "names_modifications_engines"}
                , {"number", trans("Number"), DBTFieldInfo::wtype_spinBoxInt, ""}
            },
            // Ident str:      DG90 L2.1 #273
            {
                  {"", 1}
                , {QString(" %1").arg(trans("#")), 2}
            }
        },

        /* fuels_types */
        new DBTInfo {
            "fuels_types", trans("Fuel type"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      Zhidkoe
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
            // Ident str:      Elektrodvigatel
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
            // Ident str:      Elektrodvigatel DST7
            {
                  {"", 1}
                , {" ", 2}
            }
        },

        /* injectors_types */
        new DBTInfo {
            "injectors_types", trans("Injector types"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      Type1
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
            // Ident str:      #3, chertezh: H50001082
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
            // Ident str:       DG90 L2.1 #273, toplivo: Zhidkoe
            {
                  {"", 1}
                , {QString(", %1: ").arg(trans("fuel")), 2}
            }
        },

        /* settings_names */
        new DBTInfo {
            "settings_names", trans("Names of settings"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:     Termoogranichenie
            {
                  {"", 1}
            }
        },

        /* engines_settings */
        new DBTInfo {
            "engines_settings", trans("Settings"), DBTInfo::ttype_composite,
            {
                  {"engine_id", trans("Engine's id"), DBTFieldInfo::wtype_lineEdit, "engines"}
                , {"setting_name_id", trans("Setting's name id"), DBTFieldInfo::wtype_lineEdit, "settings_names"}
                , {"setting_value", trans("Setting's value"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
                , {"serial_number", trans("Serial number"), DBTFieldInfo::wtype_not_show, ""}
            },
            // Ident str:       DG90 L2.1 #273, toplivo: Zhidkoe | Termoogranichenie: 570
            {
                  {"", 0}
                , {" | ", 1}
                , {": ", 2}
            }
        },

        /* bypass_types */
        new DBTInfo {
            "bypass_types", trans("Bypass types"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      NZ
            {
                  {"", 1}
            }
        },

        /* bypasses */
        new DBTInfo {
            "bypasses", trans("Bypasses"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"type_id", trans("Bypass type"), DBTFieldInfo::wtype_lineEdit, "bypass_types"}
                , {"S_section", trans("S section"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"draft_number", trans("Draft number"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       T907533410, NZ, S = 60
            {
                  {"", 3}
                , {QString(", "), 1}
                , {", S = ", 2}
            }
        },

        /* bypass_mount_places */
        new DBTInfo {
            "bypass_mount_places", trans("Bypasses mount places"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:      za 5 st. KVD
            {
                  {"", 1}
            }
        },

        /* engines_bypasses */
        new DBTInfo {
            "engines_bypasses", trans("Bypasses"), DBTInfo::ttype_composite,
            {
                  {"engine_id", trans("Engine"), DBTFieldInfo::wtype_lineEdit, "engines"}
                , {"bypass_id", trans("Bypass"), DBTFieldInfo::wtype_lineEdit, "bypasses"}
                , {"mount_place_id", trans("Mount place"), DBTFieldInfo::wtype_lineEdit, "bypass_mount_places"}
                , {"quantity", trans("Quantity"), DBTFieldInfo::wtype_spinBoxInt, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
                , {"serial_number", trans("Serial number"), DBTFieldInfo::wtype_not_show, ""}
            },
            // Ident str:       DG90 L2.1 #273, toplivo: Zhidkoe | 3 KPR za 5 st. KVD
            {
                  {"", 0}
                , {" | ", 3}
                , {QString(" %1 ").arg(trans("bypass")), 2}
            }
        },

        /* alg_parameters_names */
        new DBTInfo {
            "alg_parameters_names", trans("Algorithm's parameters names"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       Vklyuchenie II skorosti
            {
                  {"", 1}
            }
        },

        /* on_off_units */
        new DBTInfo {
            "on_off_units", trans("Units"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       ob/min
            {
                  {"", 1}
            }
        },

        /* on_off_parameters */
        new DBTInfo {
            "on_off_parameters", trans("Parameters"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"unit_id", trans("Unit"), DBTFieldInfo::wtype_lineEdit, "on_off_units"}
            },
            // Ident str:       Chastota vrasheniya rotora TKVD, ob/min
            {
                  {"", 1}
                , {", ", 2}
            }
        },

        /* engines_algorithms */
        new DBTInfo {
            "engines_algorithms", trans("Algorithms"), DBTInfo::ttype_composite,
            {
                  {"engine_id", trans("Engine"), DBTFieldInfo::wtype_lineEdit, "engines"}
                , {"parameter_id", trans("Parameter"), DBTFieldInfo::wtype_lineEdit, "alg_parameters_names"}
                , {"switching_on_id", trans("Switching-on"), DBTFieldInfo::wtype_lineEdit, "on_off_parameters"}
                , {"switching_on_value", trans("Switching-on value"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"switching_off_id", trans("Switching-off"), DBTFieldInfo::wtype_lineEdit, "on_off_parameters"}
                , {"switching_off_value", trans("Switching-off value"), DBTFieldInfo::wtype_spinBoxDouble, ""}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
                , {"serial_number", trans("Serial number"), DBTFieldInfo::wtype_not_show, ""}
            },
            // Ident str:       Algorithm: DG90 L2.1 #273, toplivo: Zhidkoe
            {
                  {"Algorithm: ", 0}
            }
        },

        /* documents_types */
        new DBTInfo {
            "documents_types", trans("Documents types"), DBTInfo::ttype_simple,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       Tehnicheskaya spravka
            {
                  {"", 1}
            }
        },

        /* documents */
        new DBTInfo {
            "documents", trans("Documents"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"name", trans("Name"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"number", trans("Number"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"type_id", trans("Type"), DBTFieldInfo::wtype_lineEdit, "documents_types"}
                , {"file_reference", trans("File reference"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       Tehnicheskaya spravka: TS ZHAKI.102.118-2016
            {
                  {"", 3}
                , {": ", 2}
            }
        },

        /* engines_documents */
        new DBTInfo {
            "engines_documents", trans("Documents"), DBTInfo::ttype_composite,
            {
                  {"engine_id", trans("Engine"), DBTFieldInfo::wtype_lineEdit, "engines"}
                , {"document_id", trans("Document"), DBTFieldInfo::wtype_lineEdit, "documents"}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
                , {"serial_number", trans("Serial number"), DBTFieldInfo::wtype_not_show, ""}
            },
            // Ident str:       DG90 L2.1 #273, toplivo: Zhidkoe | Tehnicheskaya spravka: TS ZHAKI.102.118-2016
            {
                  {"", 0}
                , {" | ", 1}
            }
        },

        /* graphs_parameters_type */
        new DBTInfo {
            "graphs_parameters_type", trans("Graph's parameters types"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"symbol", trans("Symbol"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"full_name", trans("Full name"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"units", trans("Units"), DBTFieldInfo::wtype_lineEdit, ""}
            },
            // Ident str:       n1, ob/min
            {
                  {"", 1}
                , {", ", 3}
            }
        },

        /* graphs_parameters_values */
        new DBTInfo {
            "graphs_parameters_values", trans("Graph's parameters values"), DBTInfo::ttype_complex,
            {
                  {"id", trans("Id"), DBTFieldInfo::wtype_lineEdit, ""}
                , {"par_type_id", trans("Parameter type"), DBTFieldInfo::wtype_lineEdit, "graphs_parameters_type"}
                , {"par_values", trans("Parameter values"), DBTFieldInfo::wtype_plainTextEdit, ""}
            },
            // Ident str:       [n1, ob/min]
            {
                  {"[", 1}
                , {"]", DBTInfo::NFIELD_STRING_AFTER}
            }
        },

        /* engines_graphs */
        new DBTInfo {
            "engines_graphs", trans("Graphs"), DBTInfo::ttype_composite,
            {
                  {"engine_id", trans("Engine"), DBTFieldInfo::wtype_lineEdit, "engines"}
                , {"par_x_id", trans("X"), DBTFieldInfo::wtype_lineEdit, "graphs_parameters_values"}
                , {"par_y_id", trans("Y"), DBTFieldInfo::wtype_lineEdit, "graphs_parameters_values"}
                , {"comments", trans("Comments"), DBTFieldInfo::wtype_plainTextEdit, ""}
                , {"serial_number", trans("Serial number"), DBTFieldInfo::wtype_not_show, ""}
            },
            // Ident str:       DG90 L2.1 #273, toplivo: Zhidkoe | [n1, ob/min] = f([t, s])
            {
                  {"", 0}
                , {" | ", 1}
                , {" = f(", 2}
                , {")", DBTInfo::NFIELD_STRING_AFTER}
            }
        },
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
