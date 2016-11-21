#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>

/*
 * The form for data input in the database
 */
namespace Ui { class FormDataInput; }
class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

signals:
    void sigInsertNew();
    void sigDeleteRow();
    void sigSaveAll();
    void sigRefreshAll();
//    void sigRevertChanges();
    void sigChangeMapperIndex(int index);
    void sigWrongIdEntered();

private:
    void setMainControls();
    void setDataNavigation();
    void setLEid();

    // TODO: use the std::unique_ptr after debugging
    Ui::FormDataInput *m_ui;
};

#endif // FORM_DATA_INPUT_H
