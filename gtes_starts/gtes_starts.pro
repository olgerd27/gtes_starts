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
    common/db_info.cpp \
    common/common_defines.cpp \
    model/custom_sql_table_model.cpp \
    model/sql_gen/generator_dbt_data.cpp \
    model/sql_gen/storage_gen_data.cpp \
    model/selection_allower.cpp \
    widgets/reimplemented_widgets.cpp \
    widgets/fl_widgets.cpp \
    dbt_editor/edit_ui_creator.cpp \
    dbt_editor/table_view_ds.cpp \
    widgets/widget_mapper.cpp \
    model/proxy_filter_model.cpp \
    model/proxy_decor_model.cpp

HEADERS  += \
    appforms/main_window.h \
    appforms/form_queries.h \
    appforms/form_data_input.h \
    appforms/form_options.h \
    dbt_editor/dbt_editor.h \
    common/db_info.h \
    common/common_defines.h \
    model/custom_sql_table_model.h \
    model/sql_gen/generator_dbt_data.h \
    model/sql_gen/storage_gen_data.h \
    model/selection_allower.h \
    widgets/reimplemented_widgets.h \
    widgets/fl_widgets.h \
    dbt_editor/edit_ui_creator.h \
    dbt_editor/table_view_ds.h \
    widgets/widget_mapper.h \
    model/proxy_filter_model.h \
    model/proxy_decor_model.h

FORMS    += \
    appforms/main_window.ui \
    appforms/form_queries.ui \
    appforms/form_data_input.ui \
    appforms/form_options.ui \
    dbt_editor/dbt_editor.ui

OTHER_FILES += \
    _aux/requirements.txt

RESOURCES += \
    images_resources.qrc

RC_FILE += icoset.rc
