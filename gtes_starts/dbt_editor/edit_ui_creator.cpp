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

LabelCreator *createLabelCreator(LabelCreator::CreatorsTypes type)
{
    LabelCreator *creator = 0;
    switch (type) {
    case LabelCreator::ctype_descriptive:
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
 * UI creator
 */
// AbstractUICreator
AbstractUICreator::~AbstractUICreator()
{ }

// EditUICreator
EditUICreator::EditUICreator(const dbi::DBTInfo *tblInfo, QDataWidgetMapper *mapper, QObject *sigReceiver, const char *slotMember)
    : m_tableInfo(tblInfo)
    , m_mapper(mapper)
    , m_lblCreator( createLabelCreator(LabelCreator::ctype_descriptive) ) // get descriptive label creator
    , m_sigReceiver(sigReceiver)
    , m_slotMember(slotMember)
{ }

EditUICreator::~EditUICreator()
{ }

void EditUICreator::createUI(QWidget *parent)
{
    /*
     * The Fabric Method pattern uses only for create the description label.
     * Creating of an others widgets performs by calling simplies methods,
     * bacause they have too many features, that is hard to implement with help of OOP.
     */
    QGridLayout *layout = new QGridLayout(parent);
    int rowfWgtSpan = 1;
    QWidget *fWgt = 0;
    QWidget *pbWgt = 0;
    for (int field = 0; field < m_tableInfo->tableDegree(); ++field) {
        const auto &dbtField = m_tableInfo->fieldByIndex(field);

        // create label and put to the layout
        layout->addWidget(m_lblCreator->create(dbtField.m_nameInUI), field, 0);

        // create field widget and put to the layout
        fWgt = createFieldWidget( dbtField.m_widgetType, dbtField.isKey(), m_sigReceiver, m_slotMember );
        m_mapper->addMapping(fWgt, field + 1); // +1 - because the proxy model add decoration icon as first column in DB table
        if (dbtField.m_widgetType == dbi::DBTFieldInfo::wtype_plainTextEdit) {
            rowfWgtSpan = 2;
            /*
             * U can use QSizePolicy::Minimum or QSizePolicy::Maximum for the QSpacerItem vertical size policy
             * for expand widget in the vertical direction. If use QSizePolicy::Fixed, the heights of
             * current spacer item and neighbour plain text edit will be set as fixed to the value, passed
             * to the QSpacerItem constructor.
             * If there are no any view in the whole window, it is need to use only the QSizePolicy::Minimum,
             * that allow in time of the window resizing expand current spacer item and neighbour plain text edit.
             */
            layout->addItem( new QSpacerItem(10, 70, QSizePolicy::Minimum, QSizePolicy::Minimum), field + 1, 0 );
        }
        else { rowfWgtSpan = 1; }
        layout->addWidget(fWgt, field, 1, rowfWgtSpan, 1);

        // create push button and put it to the layout
        pbWgt = createSEPB(dbtField.isForeign());
        if (pbWgt) layout->addWidget(pbWgt, field, 2);
    }
}

QPushButton *EditUICreator::createSEPB(bool isForeign)
{
    return isForeign ? new SelectEditPB : 0;
}
