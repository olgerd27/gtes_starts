#ifndef FORM_OPTIONS_H
#define FORM_OPTIONS_H

#include <QWidget>

namespace Ui {
    class FormOptions;
}

class FormOptions : public QWidget
{
    Q_OBJECT

public:
    explicit FormOptions(QWidget *parent = 0);
    ~FormOptions();

private:
    Ui::FormOptions *ui;
};

#endif // FORM_OPTIONS_H
