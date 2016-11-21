-- SELECT * FROM gtes_starts.engines;

USE gtes_starts;

--  Retrieve the engine fuel for the particular id value
SELECT 
    fuels_types.name
FROM
    engines,
    fuels_types
WHERE
        engines.id = 2 
    AND engines.fuel_type_id = fuels_types.id;
    
-- Retrieve combustion chambers info
SELECT 
    combustion_chambers.id, 
    combustion_chambers.draft_number
FROM
    combustion_chambers
WHERE
    3 = combustion_chambers.id;

--  Retrieve the full engine name for the foreign key id value
SELECT 
    555, 
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
    
--  Retrieve the full engine name for the primary key id value
SELECT 
    engines.id,
    names_engines.name, 
    names_modifications_engines.modification, 
    full_names_engines.number
FROM
    engines,
    names_engines,
    names_modifications_engines,
    full_names_engines
WHERE
        1 = engines.id
    AND engines.full_name_id = full_names_engines.id 
    AND full_names_engines.name_modif_id = names_modifications_engines.id 
    AND names_modifications_engines.name_id = names_engines.id;
    
-- Retrieve full engines names for all available foreign key id values
SELECT 
    engines.id,
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
   
-- Retrieve the id values of the all related tables to the full_names_engines, when is known the engines.full_name_id value
-- SELECT ???

-- Retrieve an information for the composite DB table engines_bypasses
SELECT 
    engines_bypasses.id, 
    names_engines.name, 
    names_modifications_engines.modification, 
    full_names_engines.number, 
    fuels_types.name
FROM
    engines_bypasses, 
    engines,
    names_engines, 
    names_modifications_engines, 
    full_names_engines, 
    fuels_types
WHERE
        engines_bypasses.engine_id = engines.id 
    AND engines.full_name_id = full_names_engines.id 
    AND full_names_engines.name_modif_id = names_modifications_engines.id
    AND names_modifications_engines.name_id = names_engines.id
    AND engines.fuel_type_id = fuels_types.id;

-- The table description information
DESC gtes_starts.engines_bypasses;
DESCRIBE gtes_starts.engines_bypasses;
SHOW COLUMNS FROM gtes_starts.engines_bypasses;