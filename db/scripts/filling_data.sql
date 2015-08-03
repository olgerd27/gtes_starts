# Insertion the test data to the DB

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_names.dat'
REPLACE
INTO TABLE gtes_starts.engines_names
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/fuels_types.dat'
REPLACE
INTO TABLE gtes_starts.fuels_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/start_devices_types.dat'
REPLACE
INTO TABLE gtes_starts.start_devices_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/start_devices.dat'
REPLACE
INTO TABLE gtes_starts.start_devices
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/injectors_types.dat'
REPLACE
INTO TABLE gtes_starts.injectors_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/combustion_chambers.dat'
REPLACE
INTO TABLE gtes_starts.combustion_chambers
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines.dat'
REPLACE
INTO TABLE gtes_starts.engines
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/graphs_parameters_type.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_type
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/graphs_parameters_values.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_values
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_graphs.dat'
REPLACE
INTO TABLE gtes_starts.engines_graphs
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/documents_types.dat'
REPLACE
INTO TABLE gtes_starts.documents_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/documents.dat'
REPLACE
INTO TABLE gtes_starts.documents
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_documents.dat'
REPLACE
INTO TABLE gtes_starts.engines_documents
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/alg_par_types.dat'
REPLACE
INTO TABLE gtes_starts.alg_par_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/alg_parameters_names.dat'
REPLACE
INTO TABLE gtes_starts.alg_parameters_names
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_algorithms.dat'
REPLACE
INTO TABLE gtes_starts.engines_algorithms
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypass_types.dat'
REPLACE
INTO TABLE gtes_starts.bypass_types
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypasses.dat'
REPLACE
INTO TABLE gtes_starts.bypasses
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypass_mount_places.dat'
REPLACE
INTO TABLE gtes_starts.bypass_mount_places
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_bypasses.dat'
REPLACE
INTO TABLE gtes_starts.engines_bypasses
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/settings_names.dat'
REPLACE
INTO TABLE gtes_starts.settings_names
CHARACTER SET cp1251;

LOAD DATA INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_settings.dat'
REPLACE
INTO TABLE gtes_starts.engines_settings
CHARACTER SET cp1251;

#SELECT * FROM gtes_starts.engines;