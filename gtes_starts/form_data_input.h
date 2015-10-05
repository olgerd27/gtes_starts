#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

namespace Ui {
    class FormDataInput;
}
class DBTableDialog;
class DBTableInfo;
class QSqlQueryModel;
class QDataWidgetMapper;

class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

signals:
    void sigChangeMapperIndex(int index);
    void sigWrongIdEntered();

private slots:
    void slotNeedChangeMapperIndex();
    void slotOpenDBTableDialog();
    void slotTemp();

private:
    void setPushBtnsForEditDBTables();
    void populateData();
    void setRecordsNavigation();
    DBTableDialog * createDBTableDialog(DBTableInfo *info);

    Ui::FormDataInput *ui;
    QSqlQueryModel *m_enginesModel;
    QDataWidgetMapper *m_mapper;
};

#endif // FORM_DATA_INPUT_H
