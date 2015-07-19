#ifndef FORM_GRAPHS_H
#define FORM_GRAPHS_H

#include <QWidget>

namespace Ui {
    class FormGraphs;
}

class FormGraphs : public QWidget
{
    Q_OBJECT

public:
    explicit FormGraphs(QWidget *parent = 0);
    ~FormGraphs();

private:
    Ui::FormGraphs *ui;
};

#endif // FORM_GRAPHS_H
