#ifndef FORM_DATA_INPUT_H
#define FORM_DATA_INPUT_H

#include <QWidget>
#include <memory>

namespace Ui {
    class FormDataInput;
}
namespace dbi {
    class DBTInfo;
}
class DBTEditor;
class CustomSqlTableModel;
class ProxyChoiceDecorModel;
class QDataWidgetMapper;
class SelectEditPB;
class ChangerMChTypeImpl;

/*
 * Class for change model change type.
 * The available types are defined in the MChTypeLabel class, inherited from the QLabel.
 */
class ChangerMChType : public QObject
{
    Q_OBJECT

public:
    explicit ChangerMChType(QObject *parent = 0);
    ~ChangerMChType();
    void updateModelChange(const QVariant &idPrimary, int changeType); // changeType is the MChTypeLabel::ChangeTypes
    void clearChanges();

public slots:
    void slotCheckModelChanges(const QVariant &idPrimary);

signals:
    void sigChangeChangedType(int ctype);
    void sigChangeChangedType(bool isDeleted);

private:
    std::unique_ptr<ChangerMChTypeImpl> m_pImpl; // std::unique_ptr<> needs ChangerMChType destructor implementation
};

/*
 * The form for data input in the database
 */
class FormDataInput : public QWidget
{
    Q_OBJECT

public:
    explicit FormDataInput(QWidget *parent = 0);
    ~FormDataInput();

signals:
    void sigChangeMapperIndex(int index);
    void sigInsertNew();
    void sigDeleteRow();
    void sigSaveAll();
    void sigRefreshAll();
//    void sigRevertChanges();
    void sigWrongIdEntered();
    void sigFirstRowReached(bool);
    void sigLastRowReached(bool);
    void sigChangesSubmitted(int currentIndex);

private slots:
    void slotNeedChangeMapperIndex(const QString &value);
    void slotRowIndexChanged(int row);
    void slotSubmit();
    void slotEditChildDBT();

private:
    void setMainControls();
    void setEditDBTPushButtons();
    void setEditDBTOnePB(SelectEditPB *pb, const QString &pbname, QWidget *identWidget);
    void setDataOperating();
    void setDataNavigation();
    void setModelChange();

    // TODO: use the std::unique_ptr after debugging
//    std::unique_ptr<Ui::FormDataInput> m_ui;
//    std::unique_ptr<ProxyChoiceDecorModel> m_proxyModel;
//    std::unique_ptr<QDataWidgetMapper> m_mapper;
//    std::unique_ptr<ChangerMChType> m_mchTChanger;
    Ui::FormDataInput *m_ui;
    ProxyChoiceDecorModel *m_proxyModel;
    QDataWidgetMapper *m_mapper;
    ChangerMChType *m_mchTChanger;
};

#endif // FORM_DATA_INPUT_H
