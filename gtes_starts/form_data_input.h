#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

namespace Ui {
    class FormDataInput;
}

class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

private slots:
    void slotEditEnginesNames();
    void slotEditFuels();

private:
    void editSimplyDBTable(const QString &tablename);

    Ui::FormDataInput *ui;
};

#endif // FORM_DATA_INPUT_H
