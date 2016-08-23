#include <QDebug>

#include "edit_ui_creator.h"
#include "../common/db_info.h"
#include "../common/common_defines.h"
#include "../common/fl_widgets.h"
#include "../common/reimplemented_widgets.h"

/*
 * DLabelCreator
 */
QLabel * DLabelCreator::create(const QString &text) const
{
    QLabel *lbl = new QLabel(text + ":"); // TODO: use smart pointer
    lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    return lbl;
}

AbstractLabelCreator *createLabelCreator(AbstractLabelCreator::CreatorsTypes type)
{
    AbstractLabelCreator *creator = 0;
    switch (type) {
    case AbstractLabelCreator::ctype_descriptive:
        creator = new DLabelCreator;
        break;
    default:
        ASSERT_DBG( false, cmmn::MessageException::type_critical, QObject::tr("Error label type"),
                    QObject::tr("Cannot create the label for UI - unknow type"),
                    QString("createLabelCreator()") );
        return 0;
    }
    return creator;
}

/*
 * Widget Placer
 */
// AbstractWidgetPlacer
AbstractWidgetPlacer::AbstractWidgetPlacer(QGridLayout *layout, PlacerTypes type)
    : m_layout(layout)
    , m_ptype(type)
{ }

AbstractWidgetPlacer::PlacerTypes AbstractWidgetPlacer::placerType() const
{
    return m_ptype;
}

AbstractWidgetPlacer::~AbstractWidgetPlacer()
{ }

// CommonWidgetPlacer
CommonWidgetPlacer::CommonWidgetPlacer(QGridLayout *layout)
    : AbstractWidgetPlacer(layout, ptype_common)
{ }

CommonWidgetPlacer::~CommonWidgetPlacer()
{ }

void CommonWidgetPlacer::placeWidget(int row, int column, QWidget *wgt)
{
    if (wgt) m_layout->addWidget(wgt, row, column, 1, 1);
}

// WithSpacerWidgetPlacer
WithSpacerWidgetPlacer::WithSpacerWidgetPlacer(QGridLayout *layout)
    : AbstractWidgetPlacer(layout, ptype_withSpacer)
{ }

WithSpacerWidgetPlacer::~WithSpacerWidgetPlacer()
{ }

void WithSpacerWidgetPlacer::placeWidget(int row, int column, QWidget *wgt)
{
    /*
     * U can use QSizePolicy::Minimum or QSizePolicy::Maximum for the QSpacerItem vertical size policy
     * for expand widget in the vertical direction. If use QSizePolicy::Fixed, the heights of
     * current spacer item and neighbour plain text edit will be set as fixed to the value, passed
     * to the QSpacerItem constructor.
     * If there are no any view in the whole window, it is need to use only the QSizePolicy::Minimum,
     * that allow in time of the window resizing expand current spacer item and neighbour plain text edit.
     */
    if (wgt) {
        m_layout->addItem( new QSpacerItem(10, 70, QSizePolicy::Minimum, QSizePolicy::Minimum), row + 1, column - 1 );
        m_layout->addWidget( wgt, row, column, 2, 1 );
    }
}

std::shared_ptr<AbstractWidgetPlacer> & createWidgetPlacer(const dbi::DBTFieldInfo &fieldInfo,
                                                           QGridLayout *layout,
                                                           std::shared_ptr<AbstractWidgetPlacer> &currentPlacer)
{
    bool isPlainTextEdit = fieldInfo.m_widgetType == dbi::DBTFieldInfo::wtype_plainTextEdit;
    if ( isPlainTextEdit && ( currentPlacer == 0 ||
                              currentPlacer->placerType() != AbstractWidgetPlacer::ptype_withSpacer ) )
        currentPlacer.reset( new WithSpacerWidgetPlacer(layout) );
    else if ( !isPlainTextEdit && ( currentPlacer == 0 ||
                                    currentPlacer->placerType() != AbstractWidgetPlacer::ptype_common ) )
        currentPlacer.reset( new CommonWidgetPlacer(layout) );
    return currentPlacer;
}

/*
 * UI creator
 */
// AbstractUICreator
AbstractUICreator::~AbstractUICreator()
{ }

// EditUICreator
EditUICreator::EditUICreator(const dbi::DBTInfo *tblInfo, QDataWidgetMapper *mapper, QObject *parent)
    : QObject(parent)
    , m_tableInfo(tblInfo)
    , m_mapper(mapper)
    , m_lblCreator( createLabelCreator(AbstractLabelCreator::ctype_descriptive) ) // get descriptive label creator
    , m_dataSender(new WidgetDataSender(this))
{
    connect(m_dataSender.get(), SIGNAL(sigSendLostFocusWidgetData(QWidget*,QString)),
            this, SIGNAL(sigWidgetFocusLost(QWidget*,QString))); // transmit widget and its data when this widget lose the focus
}

EditUICreator::~EditUICreator()
{ }

void EditUICreator::createUI(QWidget *parent)
{
    QGridLayout *layout = new QGridLayout(parent);
    QWidget *fWgt = 0;
    SelectEditPB *pbse = 0;
    std::shared_ptr<AbstractWidgetPlacer> fwPlacer;
    std::shared_ptr<AbstractWidgetPlacer> cmmnPlacer(new CommonWidgetPlacer(layout)); // description label and push button placer
    for (int field = 0; field < m_tableInfo->tableDegree(); ++field) {
        const auto &dbtField = m_tableInfo->fieldByIndex(field);

        // create label and place to the layout
        cmmnPlacer->placeWidget(field, 0, m_lblCreator->create(dbtField.m_nameInUI));

        // create field widget and place to the layout
        fWgt = createFieldWidget( dbtField.m_widgetType, dbtField.isKey(), m_dataSender.get() );
        m_mapper->addMapping(fWgt, field + 1); // +1 - because the proxy model add decoration icon as first column in DB table
        fwPlacer = createWidgetPlacer(dbtField, layout, fwPlacer);
        fwPlacer->placeWidget(field, 1, fWgt);

        // create select/edit push button and place it to the layout
        pbse = createSEPB(dbtField.isForeign());
        cmmnPlacer->placeWidget(field, 2, pbse);
        setEditChildPB(pbse, dbtField.m_relationDBTable, field);
    }
}

SelectEditPB *EditUICreator::createSEPB(bool canCreate)
{
    return canCreate ? new SelectEditPB : 0;
}

void EditUICreator::setEditChildPB(SelectEditPB *pb, const QString &childTableName, int fieldNo)
{
    if (!pb) return;
    pb->setDBTableName(childTableName);
    pb->setFieldNumber(fieldNo);
    connect(pb, &SelectEditPB::clicked, this, &EditUICreator::slotTransmitSEPBInfo);
}

void EditUICreator::slotTransmitSEPBInfo()
{
    SelectEditPB *pbSEDBT = qobject_cast<SelectEditPB *>(sender());
    ASSERT_DBG( pbSEDBT, cmmn::MessageException::type_critical, tr("Invalid push button"),
                tr("Cannot define the clicked push button."),
                QString("EditUICreator::setEditDBTOnePB => [alfa]") );

    const dbi::DBTInfo *tableInfo = DBINFO.tableByName(pbSEDBT->DBTableName());
    ASSERT_DBG( tableInfo, cmmn::MessageException::type_critical, tr("Invalid push button"),
                tr("Cannot define the clicked push button. Cannot obtain the database table info."),
                QString("EditUICreator::setEditDBTOnePB => [alfa]") );

    emit sigSEPBClicked(tableInfo, pbSEDBT->fieldNo());
}