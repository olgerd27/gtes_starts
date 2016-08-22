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

void cmmn::throwConversionIdErrorMsg(bool isConv, const QVariant &varId)
{
    throwErrorMsg(isConv, cmmn::MessageException::type_critical, QObject::tr("Conversion error"),
                  QObject::tr("Cannot convert the QVariant type value \"%1\" to the id values type.").arg(varId.toString()),
                  "cmmn::throwConversionIdErrorMsg");
}


void cmmn::throwErrorMsg(bool isSuccess, MessageException::MessageTypes msgType,
                         const QString &title, const QString &what, const QString &where)
{
    /*
     * TODO: if the msgType is type_critical or type_fatal, add to the end of message body the next phrase:
     * "\n\nPlease consult with the application developer for fixing this problem."
     */
    if (!isSuccess)
        throw cmmn::MessageException( msgType, title, what, where );
}
