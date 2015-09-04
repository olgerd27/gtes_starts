#-------------------------------------------------
#
# Project created by QtCreator 2015-07-15T11:00:15
#
#-------------------------------------------------

QT += core gui widgets sql
TARGET = gtes_starts
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        main_window.cpp \
    form_queries.cpp \
    form_data_input.cpp \
    form_options.cpp \
    core_app.cpp \
    dlgs/db_table_dialog.cpp \
    dlgs/simple_db_table_dialog.cpp \
    dlgs/complex_db_table_dialog.cpp \
    db_info.cpp

HEADERS  += main_window.h \
    form_queries.h \
    form_data_input.h \
    form_options.h \
    reimplemented_widgets.h \
    core_app.h \
    dlgs/db_table_dialog.h \
    dlgs/complex_db_table_dialog.h \
    dlgs/simple_db_table_dialog.h \
    db_info.h

FORMS    += main_window.ui \
    form_queries.ui \
    form_data_input.ui \
    form_options.ui \
    dlgs/simple_db_table_dialog.ui \
    dlgs/complex_db_table_dialog.ui

OTHER_FILES += \
    _aux/requirements.txt

RESOURCES += \
    images_resources.qrc
