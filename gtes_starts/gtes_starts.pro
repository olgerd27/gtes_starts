#-------------------------------------------------
#
# Project created by QtCreator 2015-07-15T11:00:15
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gtes_starts
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp \
    form_queries.cpp \
    form_data_input.cpp \
    form_options.cpp

HEADERS  += main_window.h \
    form_queries.h \
    form_data_input.h \
    form_options.h \
    reimplemented_widgets.h

FORMS    += main_window.ui \
    form_queries.ui \
    form_data_input.ui \
    form_options.ui

OTHER_FILES += \
    _aux/requirements.txt

RESOURCES += \
    images_resources.qrc
