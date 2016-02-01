#include <QObject>
#include <QVariant>
#include "common_defines.h"

int cmmn::safeQVariantToInt(const QVariant &value)
{
    bool b = false;
    int iValue = value.toInt(&b);
    if (!b)
        throw cmmn::MessageException( cmmn::MessageException::type_critical, QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the QVariant type value \"%1\" to the integer type.")
                                      .arg(value.toString()), "safeQVariantToInt" );
    return iValue;
}


cmmn::T_id cmmn::safeQVariantToIdType(const QVariant &value)
{
    bool b = false;
    auto idValue = value.toLongLong(&b);
    if (!b)
        throw cmmn::MessageException( cmmn::MessageException::type_critical, QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the QVariant type value \"%1\" to the id values type.")
                                      .arg(value.toString()), "safeQVariantToIdType" );
    return idValue;
}
