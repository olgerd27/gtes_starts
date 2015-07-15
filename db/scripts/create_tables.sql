CREATE DATABASE IF NOT EXISTS gtes_starts;
USE gtes_starts;

# combustion chamber
CREATE TABLE IF NOT EXISTS injectors_types (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(50) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS combustion_chambers (
    id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    draft_number VARCHAR(20),
    injectors_type_id TINYINT UNSIGNED,
    igniters_quantity TINYINT UNSIGNED,
    comments VARCHAR(250),
    PRIMARY KEY (id),
    FOREIGN KEY (injectors_type_id)
        REFERENCES injectors_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# start device
CREATE TABLE IF NOT EXISTS start_devices_types (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(50) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

-- Nnom - power nominal
-- n_nom - rotation speed nominal
-- kp - overloading coefficient
-- f1 - first speed
-- f2 - second speed
CREATE TABLE IF NOT EXISTS start_devices (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    device_type_id TINYINT UNSIGNED,
    model VARCHAR(30) NOT NULL,
    Nnom FLOAT,
    n_nom FLOAT,
    kp FLOAT,
    f1 FLOAT,
    f2 FLOAT,
    comments VARCHAR(250),
    PRIMARY KEY (id),
    FOREIGN KEY (device_type_id)
        REFERENCES start_devices_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# engines
CREATE TABLE IF NOT EXISTS engines_names (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(20) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS fuels_types (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(20) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines (
    id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name_id SMALLINT UNSIGNED NOT NULL,
    number SMALLINT UNSIGNED,
    fuel_type_id TINYINT UNSIGNED NOT NULL,
    combustion_chamber_id MEDIUMINT UNSIGNED,
    start_device_id SMALLINT UNSIGNED,
    start_devices_quantity TINYINT UNSIGNED,
    comments VARCHAR(250),
    PRIMARY KEY (id),
    FOREIGN KEY (name_id)
        REFERENCES engines_names (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (fuel_type_id)
        REFERENCES fuels_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (combustion_chamber_id)
        REFERENCES combustion_chambers (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (start_device_id)
        REFERENCES start_devices (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# settings
CREATE TABLE IF NOT EXISTS settings_names (
    id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(60),
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines_settings (
    engine_id MEDIUMINT UNSIGNED NOT NULL,
    setting_name_id MEDIUMINT UNSIGNED NOT NULL,
    setting_value FLOAT,
    comments VARCHAR(250),
    PRIMARY KEY (engine_id, setting_name_id),
    FOREIGN KEY (engine_id)
        REFERENCES engines (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (setting_name_id)
        REFERENCES settings_names (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# air bypass valve (ABV)
CREATE TABLE IF NOT EXISTS abv_types (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(30),
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

-- S_section - area of ABV section (cm2)
CREATE TABLE IF NOT EXISTS abv (
    id MEDIUMINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    type_id TINYINT UNSIGNED,
    S_section FLOAT,
    draft_number VARCHAR(20),
    PRIMARY KEY (id),
    FOREIGN KEY (type_id)
        REFERENCES abv_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS abv_mount_places (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(40),
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines_abv (
    engine_id MEDIUMINT UNSIGNED NOT NULL,
    abv_id MEDIUMINT UNSIGNED NOT NULL,
    mount_place_id TINYINT UNSIGNED NOT NULL,
    quantity TINYINT,
    PRIMARY KEY (engine_id, abv_id, mount_place_id),
    FOREIGN KEY (engine_id)
        REFERENCES engines (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (abv_id)
        REFERENCES abv (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (mount_place_id)
        REFERENCES abv_mount_places (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# algorithms
CREATE TABLE IF NOT EXISTS alg_parameters_names (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(100) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS alg_par_types (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(100) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines_algorithms (
    engine_id MEDIUMINT UNSIGNED NOT NULL,
    parameter_id SMALLINT UNSIGNED NOT NULL,
    switching_on_id SMALLINT UNSIGNED,
    switching_on_value FLOAT,
    switching_off_id SMALLINT UNSIGNED,
    switching_off_value FLOAT,
    comments VARCHAR(250),
    PRIMARY KEY (engine_id, parameter_id),
    FOREIGN KEY (engine_id)
        REFERENCES engines (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (parameter_id)
        REFERENCES alg_parameters_names (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (switching_on_id)
        REFERENCES alg_par_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (switching_off_id)
        REFERENCES alg_par_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# documents
CREATE TABLE IF NOT EXISTS documents_types (
    id TINYINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(50) NOT NULL,
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS documents (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    name VARCHAR(150) NOT NULL,
    number VARCHAR(20) NOT NULL,
    type_id TINYINT UNSIGNED,
    file_reference VARCHAR(150),
    PRIMARY KEY (id),
    FOREIGN KEY (type_id)
        REFERENCES documents_types (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines_documents (
    engine_id MEDIUMINT UNSIGNED NOT NULL,
    document_id SMALLINT UNSIGNED NOT NULL,
    comments VARCHAR(250),
    PRIMARY KEY (engine_id, document_id),
    FOREIGN KEY (engine_id)
        REFERENCES engines (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (document_id)
        REFERENCES documents (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

# graphs
CREATE TABLE IF NOT EXISTS graphs_parameters_type (
    id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE,
    symbol VARCHAR(15),
    full_name VARCHAR(70) NOT NULL,
    units VARCHAR(40),
    PRIMARY KEY (id)
)  ENGINE InnoDB CHARACTER SET cp1251;

-- what type use for storing float values array in the field "par_values"??
CREATE TABLE IF NOT EXISTS graphs_parameters_values (
    id MEDIUMINT UNSIGNED NOT NULL,
    par_type_id SMALLINT UNSIGNED NOT NULL,
    par_values BLOB,
    PRIMARY KEY (id),
    FOREIGN KEY (par_type_id)
        REFERENCES graphs_parameters_type (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;

CREATE TABLE IF NOT EXISTS engines_graphs (
    engine_id MEDIUMINT UNSIGNED NOT NULL,
    par_x_id MEDIUMINT UNSIGNED NOT NULL,
    par_y_id MEDIUMINT UNSIGNED NOT NULL,
    comments VARCHAR(250),
    PRIMARY KEY (engine_id, par_x_id, par_y_id),
    FOREIGN KEY (engine_id)
        REFERENCES engines (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (par_x_id)
        REFERENCES graphs_parameters_values (id)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (par_y_id)
        REFERENCES graphs_parameters_values (id)
        ON DELETE RESTRICT ON UPDATE CASCADE
)  ENGINE InnoDB CHARACTER SET cp1251;
