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

#include <QDebug>
bool cmmn::safeQVariantToIdType(const QVariant &value, T_id &id)
{
    bool bOk = false;
    id = value.toLongLong(&bOk);
    return bOk;
}


void cmmn::throwConversionIdErrorMsg(bool bConv, const QVariant &varId)
{
    qDebug() << "throwConversionIdErrorMsg() 1";
    if (!bConv) {
        qDebug() << "throwConversionIdErrorMsg() 2";
        QString strTitle = QObject::tr("Conversion error");
        qDebug() << "throwConversionIdErrorMsg() 3";
        QString strId = varId.toString();
        qDebug() << "throwConversionIdErrorMsg() 4";
        QString strMsg = QObject::tr("Cannot convert the QVariant type value \"%1\" to the id values type.").arg(strId);
        qDebug() << "throwConversionIdErrorMsg() 5";
        cmmn::MessageException me( cmmn::MessageException::type_critical, strTitle, strMsg, "cmmn::throwConversionIdErrorMsg" );
        qDebug() << "throwConversionIdErrorMsg() 6";
        throw me;
        qDebug() << "throwConversionIdErrorMsg() 7";
    }
    qDebug() << "throwConversionIdErrorMsg() 8";
}
