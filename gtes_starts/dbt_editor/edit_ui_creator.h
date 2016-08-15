#ifndef EDIT_UI_CREATOR_H
#define EDIT_UI_CREATOR_H

#include <memory>
#include <QLabel>
#include <QPushButton>
#include <QDataWidgetMapper>
#include <QGridLayout>

namespace dbi {
    class DBTInfo;
    class DBTFieldInfo;
}

// --- Create descriptive label ---
// Descriptive label - a label that store description of data, stored in the neighbour widget.
// The abstract descriptive label creator
class LabelCreator
{
public:
    enum CreatorsTypes {
        ctype_descriptive   // descriptive label type
    };

    virtual QLabel * create(const QString &text) const = 0;
};

// Default descriptive label creator
class DLabelCreator : public LabelCreator
{
public:
    virtual QLabel *create(const QString &text) const;
};

// Fabric method - return specific dlabel creator by the dlabel creator type
LabelCreator * createLabelCreator(LabelCreator::CreatorsTypes type);

// --- Create UI ---
// Abstract UI creator
class AbstractUICreator
{
public:
    virtual void createUI(QWidget *parent) = 0;
    virtual ~AbstractUICreator() = 0;
};

// The "edit" UI creator
class EditUICreator : public AbstractUICreator
{
public:
    EditUICreator(const dbi::DBTInfo *tblInfo, QDataWidgetMapper *mapper,
                  QObject *sigReceiver = 0, const char *slotMember = 0);
    virtual ~EditUICreator();
    virtual void createUI(QWidget *parent);

private:
    QPushButton * createSEPB(bool isForeign);

    const dbi::DBTInfo *m_tableInfo;
    QDataWidgetMapper *m_mapper;
    std::unique_ptr<LabelCreator> m_lblCreator;
    QObject *m_sigReceiver;
    const char *m_slotMember;
};

#endif // EDIT_UI_CREATOR_H
