# Insertion the test data to the DB
# Windows: D:/work/Starts_data_base/development/project/db/scripts/test_data
# Linux: ~/Programming/qt/projects/Starts_data_base/development/project/db/scripts/test_data

# --- engines names ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/names_engines.dat'
REPLACE
INTO TABLE gtes_starts.names_engines
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/names_modifications_engines.dat'
REPLACE
INTO TABLE gtes_starts.names_modifications_engines
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/full_names_engines.dat'
REPLACE
INTO TABLE gtes_starts.full_names_engines
CHARACTER SET cp1251;

# --- fuel ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/fuels_types.dat'
REPLACE
INTO TABLE gtes_starts.fuels_types
CHARACTER SET cp1251;

# --- start devices ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/start_devices_types.dat'
REPLACE
INTO TABLE gtes_starts.start_devices_types
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/start_devices.dat'
REPLACE
INTO TABLE gtes_starts.start_devices
CHARACTER SET cp1251;

# --- combustion chambers ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/injectors_types.dat'
REPLACE
INTO TABLE gtes_starts.injectors_types
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/combustion_chambers.dat'
REPLACE
INTO TABLE gtes_starts.combustion_chambers
CHARACTER SET cp1251;

# --- engines ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines.dat'
REPLACE
INTO TABLE gtes_starts.engines
CHARACTER SET cp1251;

# --- graphs ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/graphs_parameters_type.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_type
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/graphs_parameters_values.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_values
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_graphs.dat'
REPLACE
INTO TABLE gtes_starts.engines_graphs
CHARACTER SET cp1251;

# --- documents ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/documents_types.dat'
REPLACE
INTO TABLE gtes_starts.documents_types
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/documents.dat'
REPLACE
INTO TABLE gtes_starts.documents
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_documents.dat'
REPLACE
INTO TABLE gtes_starts.engines_documents
CHARACTER SET cp1251;

# --- algorithms ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/on_off_units.dat'
REPLACE
INTO TABLE gtes_starts.on_off_units
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/on_off_parameters.dat'
REPLACE
INTO TABLE gtes_starts.on_off_parameters
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/alg_parameters_names.dat'
REPLACE
INTO TABLE gtes_starts.alg_parameters_names
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_algorithms.dat'
REPLACE
INTO TABLE gtes_starts.engines_algorithms
CHARACTER SET cp1251;

# --- bypasses ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypass_types.dat'
REPLACE
INTO TABLE gtes_starts.bypass_types
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypasses.dat'
REPLACE
INTO TABLE gtes_starts.bypasses
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/bypass_mount_places.dat'
REPLACE
INTO TABLE gtes_starts.bypass_mount_places
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_bypasses.dat'
REPLACE
INTO TABLE gtes_starts.engines_bypasses
CHARACTER SET cp1251;

# --- settings ---
LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/settings_names.dat'
REPLACE
INTO TABLE gtes_starts.settings_names
CHARACTER SET cp1251;

LOAD DATA LOCAL INFILE 'D:/work/Starts_data_base/development/project/db/scripts/test_data/engines_settings.dat'
REPLACE
INTO TABLE gtes_starts.engines_settings
CHARACTER SET cp1251;
