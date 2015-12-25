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
    appforms/main_window.cpp \
    appforms/form_queries.cpp \
    appforms/form_data_input.cpp \
    appforms/form_options.cpp \
    dbt_editor/dbt_editor.cpp \
    dbt_editor/simple_dbt_editor.cpp \
    dbt_editor/complex_dbt_editor.cpp \
    common/db_info.cpp \
    common/reimplemented_widgets.cpp \
    common/common_defines.cpp \
    datagen/custom_sql_table_model.cpp \
    datagen/dbt_data_generator.cpp \
    datagen/gen_data_storage.cpp

HEADERS  += \
    appforms/main_window.h \
    appforms/form_queries.h \
    appforms/form_data_input.h \
    appforms/form_options.h \
    dbt_editor/dbt_editor.h \
    dbt_editor/simple_dbt_editor.h \
    dbt_editor/complex_dbt_editor.h \
    common/db_info.h \
    common/reimplemented_widgets.h \
    common/common_defines.h \
    datagen/custom_sql_table_model.h \
    datagen/dbt_data_generator.h \
    datagen/gen_data_storage.h

FORMS    += \
    appforms/main_window.ui \
    appforms/form_queries.ui \
    appforms/form_data_input.ui \
    appforms/form_options.ui \
    dbt_editor/complex_dbt_editor.ui \
    dbt_editor/simple_dbt_editor.ui \
    dbt_editor/simple_dbt_editor2.ui

OTHER_FILES += \
    _aux/requirements.txt

RESOURCES += \
    images_resources.qrc

RC_FILE += icoset.rc
