#SELECT * FROM gtes_starts.engines;

USE gtes_starts;

# Retrieve the engine fuel for the particular id value
SELECT 
    fuels_types.name
FROM
    engines,
    fuels_types
WHERE
        engines.id = 2 
    AND engines.fuel_type_id = fuels_types.id;

# Retrieve the engine name for the particular id value
SELECT 
    names_engines.name
FROM
    engines,
    names_engines,
    names_modifications_engines,
    identification_data_engines
WHERE
        engines.id = 3 
    AND engines.identification_id = identification_data_engines.id 
    AND identification_data_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;

# Retrieve full engine name for the particular id value
SELECT 
    names_engines.name, 
    names_modifications_engines.modification, 
    identification_data_engines.number
FROM
    names_engines,
    names_modifications_engines,
    identification_data_engines
WHERE
        identification_data_engines.id = 3 
    AND identification_data_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;
    
# Retrieve full engines names for all available id values
SELECT 
    names_engines.name, 
    names_modifications_engines.modification, 
    identification_data_engines.number
FROM
    engines,
    names_engines,
    names_modifications_engines,
    identification_data_engines
WHERE
        engines.identification_id = identification_data_engines.id 
    AND identification_data_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;
