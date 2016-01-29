#ifndef FORM_OPTIONS_H
#define FORM_OPTIONS_H

#include <memory>
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
    std::unique_ptr<Ui::FormOptions> m_ui;
};

#endif // FORM_OPTIONS_H
