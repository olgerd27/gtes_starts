#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#include <sstream>
#include <QString>

namespace cmmn {
    /*
     * Class for dynamical generation of exceptions (STL library version).
     * This class can accept data with any type, that can to be added to std::stringstream type instance.
     */
    struct DynExceptionSTL : public std::exception
    {
        template<typename T>
        DynExceptionSTL & operator<<(const T &t)
        {
            std::stringstream ss;
            ss << t;
            m_msg += ss.str();
            return *this;
        }

        virtual const char * what() const noexcept
        {
            return m_msg.c_str();
        }

    private:
        std::string m_msg;
    };

    /*
     * Class for exceptions generation, that used for transfer error message to app point, that create message box
     */
    struct MessageException
    {
        enum MessageTypes {
              type_info
            , type_warning
            , type_critical
            , type_fatal
        };

        MessageException(MessageTypes type, const QString &title, const QString &msg, const QString &codePlace)
            : m_type(type)
            , m_title(title)
            , m_msg(msg)
            , m_codePlace(codePlace)
        {}

        inline MessageTypes type() const { return m_type; }
        inline QString title() const { return m_title; }
        inline QString message() const { return m_msg; }
        inline QString codePlace() const { return m_codePlace; } // TODO: maybe delete this. Code place shows only with help of Q_ASSERT macro

    private:
        MessageTypes m_type;
        QString m_title;
        QString m_msg;
        QString m_codePlace;
    };

    // Conversions
    typedef qlonglong T_id;
    T_id safeQVariantToIdType(const QVariant &value); // version with exception
    bool safeQVariantToIdType(const QVariant &value, cmmn::T_id &id); // no exception version
    void throwConversionIdErrorMsg(bool isConv, const QVariant &varId);
    void throwErrorMsg(bool isSuccess, cmmn::MessageException::MessageTypes msgType,
                       const QString &title, const QString &what, const QString &where);

    /*
     * The definition, that check the conversion of id value from the QVariant to the T_id type,
     * performed with help of the cmmn::safeQVariantToIdType() function.
     * The 1-th macro parameter is conversion success (bool type), the 2-th is initial QVariant-type id value.
     * If conversion cannot be realized (bAssert == false), in debug mode - calls assert macro,
     * in release - call the cmmn::throwConversionIdErrorMsg() function, that throws the
     * cmmn::MessageException type error message.
     */
#ifdef QT_NO_DEBUG
#define CHECK_ERROR_CONVERT_ID(bAssert, varId) \
    cmmn::throwConversionIdErrorMsg(bAssert, varId);
#else
#define CHECK_ERROR_CONVERT_ID(bAssert, varValue) \
    Q_ASSERT(bAssert);
#endif

// Macro's definition for error generating in the two modes: release (QT_NOT_DEBUG) and debug (else)
// The arguments qstrTitle, qstrWhat, qstrWhere must be passed in the QString type.
#ifdef QT_NO_DEBUG
#define ASSERT_DBG(bAssert, msgType, qstrTitle, qstrWhat, qstrWhere) \
    cmmn::throwErrorMsg(bAssert, msgType, qstrTitle, qstrWhat, qstrWhere);
#else
#define ASSERT_DBG(bAssert, msgType, qstrTitle, qstrWhat, qstrWhere) \
    Q_ASSERT_X(bAssert, (qstrWhere).toStdString().c_str(), (qstrWhat).toStdString().c_str());
#endif

}

#endif // COMMON_DEFINES_H
