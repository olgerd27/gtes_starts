# Delete data from the DB

DELETE FROM gtes_starts.engines_graphs;
DELETE FROM gtes_starts.graphs_parameters_values;
DELETE FROM gtes_starts.graphs_parameters_type;

DELETE FROM gtes_starts.engines_documents;
DELETE FROM gtes_starts.documents;
DELETE FROM gtes_starts.documents_types;

DELETE FROM gtes_starts.engines_algorithms;
DELETE FROM gtes_starts.alg_parameters_names;
DELETE FROM gtes_starts.alg_par_types;

DELETE FROM gtes_starts.engines_bypasses;
DELETE FROM gtes_starts.bypasses;
DELETE FROM gtes_starts.bypass_types;
DELETE FROM gtes_starts.bypass_mount_places;

DELETE FROM gtes_starts.engines_settings;
DELETE FROM gtes_starts.settings_names;

DELETE FROM gtes_starts.engines;

DELETE FROM gtes_starts.engines_names;
DELETE FROM gtes_starts.fuels_types;
DELETE FROM gtes_starts.start_devices;
DELETE FROM gtes_starts.start_devices_types;

DELETE FROM gtes_starts.combustion_chambers;
DELETE FROM gtes_starts.injectors_types;
