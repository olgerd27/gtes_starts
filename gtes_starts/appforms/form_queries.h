#ifndef FORM_QUERIES_H
#define FORM_QUERIES_H

#include <memory>
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
    std::unique_ptr<Ui::FormQueries> m_ui;
};

#endif // FORM_QUERIES_H
