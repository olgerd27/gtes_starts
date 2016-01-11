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
        inline QString codePlace() const { return m_codePlace; }

    private:
        MessageTypes m_type;
        QString m_title;
        QString m_msg;
        QString m_codePlace;
    };

    // Conversions
    typedef qlonglong T_id;
    int safeQVariantToInt(const QVariant &value);
    T_id safeQVariantToIdType(const QVariant &value);
}

#endif // COMMON_DEFINES_H
