# deleting the table gtes_starts.engines_names_numbers, when the simply drop command return error 1217
SET FOREIGN_KEY_CHECKS = 0;
DROP TABLE IF EXISTS gtes_starts.graphs_parameters_type;
SET FOREIGN_KEY_CHECKS = 1;
