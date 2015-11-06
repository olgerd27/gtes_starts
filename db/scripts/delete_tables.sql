-- DROP DATABASE IF EXISTS gtes_starts;

-- deleting the table gtes_starts.engines_names_numbers, when the simply drop command return error 1217
SET FOREIGN_KEY_CHECKS = 0;
DROP TABLE IF EXISTS gtes_starts.on_off_units;
DROP TABLE IF EXISTS gtes_starts.on_off_parameters;
DROP TABLE IF EXISTS gtes_starts.alg_parameters_names;
DROP TABLE IF EXISTS gtes_starts.bypass_mount_places;
DROP TABLE IF EXISTS gtes_starts.bypass_types;
DROP TABLE IF EXISTS gtes_starts.bypasses;
DROP TABLE IF EXISTS gtes_starts.combustion_chambers;
DROP TABLE IF EXISTS gtes_starts.documents;
DROP TABLE IF EXISTS gtes_starts.documents_types;
DROP TABLE IF EXISTS gtes_starts.engines;
DROP TABLE IF EXISTS gtes_starts.engines_algorithms;
DROP TABLE IF EXISTS gtes_starts.engines_bypasses;
DROP TABLE IF EXISTS gtes_starts.engines_documents;
DROP TABLE IF EXISTS gtes_starts.engines_graphs;
DROP TABLE IF EXISTS gtes_starts.names_engines;
DROP TABLE IF EXISTS gtes_starts.engines_settings;
DROP TABLE IF EXISTS gtes_starts.fuels_types;
DROP TABLE IF EXISTS gtes_starts.graphs_parameters_type;
DROP TABLE IF EXISTS gtes_starts.graphs_parameters_values;
DROP TABLE IF EXISTS gtes_starts.injectors_types;
DROP TABLE IF EXISTS gtes_starts.settings_names;
DROP TABLE IF EXISTS gtes_starts.start_devices;
DROP TABLE IF EXISTS gtes_starts.start_devices_types;
DROP TABLE IF EXISTS gtes_starts.names_modifications_engines;
DROP TABLE IF EXISTS gtes_starts.full_names_engines;
SET FOREIGN_KEY_CHECKS = 1;
-- 
-- DESCRIBE engines;
-- SHOW COLUMNS FROM test_SalesDept.Customers;