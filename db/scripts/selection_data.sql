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
    full_names_engines
WHERE
        engines.id = 3 
    AND engines.full_name_id = full_names_engines.id 
    AND full_names_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;

# Retrieve full engine name for the particular id value
SELECT 
    names_engines.name, 
    names_modifications_engines.modification, 
    full_names_engines.number
FROM
    names_engines,
    names_modifications_engines,
    full_names_engines
WHERE
        3 = full_names_engines.id
    AND full_names_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;
    
# Retrieve full engines names for all available foreign key id values
SELECT 
    names_engines.name, 
    names_modifications_engines.modification, 
    full_names_engines.number
FROM
    engines,
    names_engines,
    names_modifications_engines,
    full_names_engines
WHERE
        engines.full_name_id = full_names_engines.id 
    AND full_names_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;
