#ifndef CORE_APP_H
#define CORE_APP_H

#include <QObject>

class CoreApp : public QObject
{
    Q_OBJECT
public:
    static CoreApp & Instance()
    {
        static CoreApp theInstance;
        return theInstance;
    }
    ~CoreApp();
    const char * msg() const { return "CoreApp instance achieved"; } // TODO: delete

signals:

public slots:

private:
    explicit CoreApp(QObject *parent = 0);
    CoreApp(const CoreApp &) = delete;
    CoreApp & operator=(const CoreApp &) = delete;
};

#define COREINST CoreApp::Instance()

#endif // CORE_APP_H
