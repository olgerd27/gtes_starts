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
            type_info,
            type_warning,
            type_critical
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
    T_id safeQVariantToIdType(const QVariant &value);
    bool safeQVariantToIdType(const QVariant &value, cmmn::T_id &id);
    void throwConversionIdErrorMsg(bool bConv, const QVariant &varId);

    /*
     * The definition, that check the conversion the id value from the QVariant to the T_id type,
     * performed with help of the cmmn::safeQVariantToIdType() function.
     * The 1-th macro parameter is conversion success, the 2-th is initial QVariant-type id value.
     * If conversion cannot be realized (bOk == false), in debug mode - calls assert macro,
     * in release - call the cmmn::throwConversionIdErrorMsg() function, that throws the cmmn::MessageException type error message.
     */
#ifdef QT_NO_DEBUG
#define CHECK_ERROR_CONVERT_ID(bOk, varId) \
    cmmn::throwConversionIdErrorMsg(bOk, varId);
#else
#define CHECK_ERROR_CONVERT_ID(bOk, varValue) \
    Q_ASSERT(bOk);
#endif

    // TODO: replace this macro to the macro, that throw an exception in the release mode or call the Q_ASSERT_X macro in the debug mode
#ifdef QT_NO_DEBUG
#define ASSERT_DBG(b) \
    ;
#else
#define ASSERT_DBG(b) \
    Q_ASSERT(b);
#endif

}

#endif // COMMON_DEFINES_H
