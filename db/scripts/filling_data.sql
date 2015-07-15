LOAD DATA INFILE 'D:/work/Starts_data_base/project/gtes_starts/db/scripts/data__graphs_parameters_type.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_type
CHARACTER SET cp1251;
SELECT * FROM gtes_starts.graphs_parameters_type;

LOAD DATA INFILE 'D:/work/Starts_data_base/project/gtes_starts/db/scripts/data__graphs_parameters_values.dat'
REPLACE
INTO TABLE gtes_starts.graphs_parameters_values
CHARACTER SET cp1251;
SELECT * FROM gtes_starts.graphs_parameters_values;