#include <QObject>
#include <QVariant>
#include "common_defines.h"

cmmn::T_id cmmn::safeQVariantToIdType(const QVariant &value)
{
    bool bOk = false;
    auto idValue = value.toLongLong(&bOk);
    throwConversionIdErrorMsg(bOk, value);
    return idValue;
}

bool cmmn::safeQVariantToIdType(const QVariant &value, T_id &id)
{
    bool bOk = false;
    id = value.toLongLong(&bOk);
    return bOk;
}


void cmmn::throwConversionIdErrorMsg(bool bConv, const QVariant &varId)
{
    if (!bConv)
        throw cmmn::MessageException( cmmn::MessageException::type_critical, QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the QVariant type value \"%1\" to the id values type.").arg(varId.toString()),
                                      "cmmn::throwConversionIdErrorMsg" );
}
