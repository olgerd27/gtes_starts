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
    common/db_info.cpp \
    common/common_defines.cpp \
    models/src_sql/custom_sql_table_model.cpp \
    models/src_sql/generator_dbt_data.cpp \
    models/src_sql/storage_gen_data.cpp \
    widgets/reimplemented_widgets.cpp \
    widgets/fl_widgets.cpp \
    dbt_editor/dbt_editor.cpp \
    dbt_editor/edit_ui_creator.cpp \
    dbt_editor/table_view_ds.cpp \
    widgets/widget_mapper.cpp \
    models/prx_decor/proxy_decor_model.cpp \
    models/prx_decor/changes_model.cpp \
    models/prx_filter/proxy_filter_model.cpp \
    models/prx_filter/selection_allower.cpp

HEADERS  += \
    appforms/main_window.h \
    appforms/form_queries.h \
    appforms/form_data_input.h \
    appforms/form_options.h \
    common/db_info.h \
    common/common_defines.h \
    models/src_sql/custom_sql_table_model.h \
    models/src_sql/generator_dbt_data.h \
    models/src_sql/storage_gen_data.h \
    widgets/reimplemented_widgets.h \
    widgets/fl_widgets.h \
    dbt_editor/dbt_editor.h \
    dbt_editor/edit_ui_creator.h \
    dbt_editor/table_view_ds.h \
    widgets/widget_mapper.h \
    models/prx_decor/proxy_decor_model.h \
    models/prx_decor/changes_model.h \
    models/prx_filter/proxy_filter_model.h \
    models/prx_filter/selection_allower.h

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
