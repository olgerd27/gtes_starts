#ifndef FORM_QUERIES_H
#define FORM_QUERIES_H

#include <QWidget>

namespace Ui {
    class FormQueries;
}

class FormQueries : public QWidget
{
    Q_OBJECT

public:
    explicit FormQueries(QWidget *parent = 0);
    ~FormQueries();

private:
    Ui::FormQueries *ui;
};

#endif // FORM_QUERIES_H
