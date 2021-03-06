#ifndef EDIT_UI_CREATOR_H
#define EDIT_UI_CREATOR_H

#include <memory>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

namespace dbi {
    class DBTInfo;
    class DBTFieldInfo;
}
class SelectEditPB;

// === Create label ===
// The abstract label creator
class AbstractLabelCreator
{
public:
    enum CreatorsTypes {
        ctype_descriptive   // descriptive label type
    };

    virtual QLabel * create(const QString &text) const = 0;
};

// Descriptive label creator. Descriptive label - a label that store description of data, stored in the neighbour widget.
class DLabelCreator : public AbstractLabelCreator
{
public:
    virtual QLabel * create(const QString &text) const override;
};

// Fabric method - return specific dlabel creator by the dlabel creator type
AbstractLabelCreator * createLabelCreator(AbstractLabelCreator::CreatorsTypes type);

// === Widget Placer ===
// The abstract widget placer
class AbstractWidgetPlacer
{
public:
    enum PlacerTypes {
          ptype_common
        , ptype_withSpacer
    };

    AbstractWidgetPlacer(QGridLayout *layout, PlacerTypes type);
    virtual ~AbstractWidgetPlacer() = 0;
    virtual void placeWidget(int row, int column, QWidget *wgt) = 0;
    AbstractWidgetPlacer::PlacerTypes placerType() const;

protected:
    QGridLayout *m_layout;
    PlacerTypes m_ptype;
};

// Place widget in usually (common) way - to current [row, column] layout placing
class CommonWidgetPlacer : public AbstractWidgetPlacer
{
public:
    CommonWidgetPlacer(QGridLayout *layout);
    virtual ~CommonWidgetPlacer();
    virtual void placeWidget(int row, int column, QWidget *wgt) override;
};

// Place widget with adding the QSpacerItem on left side from widget
class WithSpacerWidgetPlacer : public AbstractWidgetPlacer
{
public:
    WithSpacerWidgetPlacer(QGridLayout *layout);
    virtual ~WithSpacerWidgetPlacer();
    virtual void placeWidget(int row, int column, QWidget *wgt) override;
};

/*
 * Create a some widget placer if current placer is not setted or current placer type is different from required.
 * The required placer type defines inside this function.
 */
std::shared_ptr<AbstractWidgetPlacer> &createWidgetPlacer(const dbi::DBTFieldInfo &fieldInfo,
                                                          QGridLayout *layout,
                                                          std::shared_ptr<AbstractWidgetPlacer> &currentPlacer);

// === Create UI ===
// Abstract UI creator
class AbstractUICreator
{
public:
    virtual ~AbstractUICreator() = 0;
    virtual void createUI(QWidget *parent) = 0;
};

// The "edit" UI creator
class WidgetDataSender;
class WidgetMapper;
class EditUICreator : public QObject, public AbstractUICreator
{
    Q_OBJECT

public:
    EditUICreator(const dbi::DBTInfo *tblInfo, WidgetMapper *mapper, QObject *parent = 0);
    virtual ~EditUICreator();
    virtual void createUI(QWidget *parent) override;

signals:
    void sigSEPBClicked(const dbi::DBTInfo *dbtInfo, int fieldNo); // SEPB - Select/Edit PushButton

private slots:
    void slotTransmitSEPBInfo();

private:
    void setBoldFont(QWidget *wgt);
    SelectEditPB * createSEPB(bool canCreate);
    void setEditChildPB(SelectEditPB *pb, const QString &childTableName, int fieldNo);

    const dbi::DBTInfo *m_tableInfo;
    WidgetMapper *m_mapper;
    std::unique_ptr<AbstractLabelCreator> m_lblCreator;
    QObject *m_sigReceiver;
    const char *m_slotMember;
    std::unique_ptr<WidgetDataSender> m_wgtDataSender;
};

#endif // EDIT_UI_CREATOR_H
