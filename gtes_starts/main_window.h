#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum panelsIndexes {
        index_data_input,  // Data input panel index
        index_queries,     // Queries panel index
        index_options      // Options panel index
    };

    Ui::MainWindow *ui;
};

#endif // MAIN_WINDOW_H
