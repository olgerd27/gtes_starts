#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <memory>
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

/*
 * TODO: add "Revert" action, that cancel prepared changes of data in mapped widgets, that shows data of the Engine DB table,
 * and revert state of mapped widgets data to the last save.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void debugOutput(const QString& msg); // TODO: temporary, delete later

private slots:
    void slotAboutApp();

private:
    enum PanelsIndexes {
          index_data_input  // Data input panel index
        , index_queries     // Queries panel index
        , index_options     // Options panel index
    };

    std::unique_ptr<Ui::MainWindow> m_ui;
};

#endif // MAIN_WINDOW_H
