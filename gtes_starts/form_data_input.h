#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

namespace Ui {
    class FormDataInput;
}
class DBTableDialog;
class DBTableInfo;

class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

private slots:
    void slotOpenDBTableDialog();

private:
    void setDBTableNames();
    DBTableDialog * defineDBTableDialog(DBTableInfo *info);

    Ui::FormDataInput *ui;
};

#endif // FORM_DATA_INPUT_H
