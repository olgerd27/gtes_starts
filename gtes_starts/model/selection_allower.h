#ifndef SELECTION_ALLOWER_H
#define SELECTION_ALLOWER_H

#include <QLineEdit>
#include <QTableView>

/*
 * Selection allower base class with default functionality.
 * Need for allow rows selection in the proxy model (ProxyDecorModel).
 */
class SelectionAllower : public QObject
{
    Q_OBJECT

public:
    enum SA_Types {
          type_Default
        , type_InputClick
    };

    SelectionAllower(QObject *parent);
    virtual ~SelectionAllower() { }
    virtual bool isSelectionAllowed() const;
};

/*
 * SelectionAllower_IC - allow selection depending on Input data in LineEdit and Click on cells in TableView
 */
class SelectionAllower_IC : public SelectionAllower
{
    Q_OBJECT

public:
    SelectionAllower_IC(QLineEdit *le, QTableView *tblView, QObject *parent);
    virtual ~SelectionAllower_IC() override { }
    virtual bool isSelectionAllowed() const override;

public slots:
    void slotSelectionEnded();

private:
    bool m_mousePressed;
    bool m_allowSelection;
};

#endif // SELECTION_ALLOWER_H
