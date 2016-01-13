#include <QApplication>
#include "appforms/main_window.h"

static MainWindow *pMainWnd = NULL;

void OnQDebugMsg(QtMsgType type,
                 const QMessageLogContext& ctx,
                 const QString& msg)
{
    Q_UNUSED(ctx);
    Q_UNUSED(type);
    if (pMainWnd) pMainWnd->debugOutput(msg);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    qInstallMessageHandler(OnQDebugMsg);
    MainWindow w;
    pMainWnd = &w;
    w.show();
    return app.exec();
}
