\set ON_ERROR_STOP on

SET client_min_messages TO WARNING;

CREATE OR REPLACE FUNCTION make_plpgsql()
RETURNS VOID
LANGUAGE SQL
AS $$
CREATE LANGUAGE plpgsql;
$$;
 
SELECT
    CASE
    WHEN EXISTS(
        SELECT 1
        FROM pg_catalog.pg_language
        WHERE lanname='plpgsql'
    )
    THEN NULL
    ELSE make_plpgsql() END;
 
DROP FUNCTION make_plpgsql();

-- ********************************************
-- UTILISATEURS ET ROLES POSTGRES
-- ********************************************
CREATE OR REPLACE FUNCTION create_users()
RETURNS VOID
AS $$
BEGIN
	IF NOT EXISTS (SELECT * FROM  pg_catalog.pg_user WHERE usename = 'dba') THEN
		CREATE USER dba WITH CREATEROLE ENCRYPTED PASSWORD 'md53ce0e2bc027c7d6f88f1ce14ea4cd57b';
	END IF;

	IF NOT EXISTS (SELECT * FROM  pg_catalog.pg_roles WHERE rolname = 'admin_role') THEN
		CREATE ROLE admin_role WITH CREATEROLE ROLE dba;
	END IF;

	IF NOT EXISTS (SELECT * FROM  pg_catalog.pg_roles WHERE rolname = 'writer_role') THEN
		CREATE ROLE writer_role;
	END IF;

	IF NOT EXISTS (SELECT * FROM  pg_catalog.pg_roles WHERE rolname = 'reader_role') THEN
		CREATE ROLE reader_role;
	END IF;
	
END;
$$ LANGUAGE plpgsql;

SELECT create_users();
DROP FUNCTION create_users();

GRANT TEMPORARY ON DATABASE :var_dbname TO admin_role, writer_role;

CREATE OR REPLACE VIEW users_roles AS
	SELECT
		rolname AS username,
		(SELECT count(roleid) FROM pg_auth_members
			WHERE member=pg_roles.oid AND roleid=(select oid from pg_roles where rolname='admin_role')) AS admin_role,
		(SELECT count(roleid) FROM pg_auth_members
			WHERE member=pg_roles.oid AND roleid=(select oid from pg_roles where rolname='writer_role')) AS writer_role,
		(SELECT count(roleid) FROM pg_auth_members
			WHERE member=pg_roles.oid AND roleid=(select oid from pg_roles where rolname='reader_role')) AS reader_role
	FROM
		pg_roles
	WHERE
		rolcanlogin=true AND rolsuper=false;

GRANT SELECT ON users_roles TO admin_role;

-- ********************************************
-- DATABASE_VERSION 
-- ********************************************
CREATE TABLE database_version_table (
  database_version_number varchar(12) NOT NULL,
  upgrade_date timestamp with time zone NOT NULL DEFAULT now()
);

COMMENT ON TABLE database_version_table IS 'Version de la base de données';

GRANT SELECT ON database_version_table TO PUBLIC;

INSERT INTO database_version_table (database_version_number) VALUES ('01.05.00.00');

-- ********************************************
-- UTILISATEURS 
-- ********************************************
CREATE SEQUENCE users_user_id_seq	START 3;

CREATE TABLE users_table (
  user_id bigint DEFAULT nextval('users_user_id_seq'),
  username varchar(64) NOT NULL,
  group_id bigint default NULL,
  email varchar(128) NOT NULL,
  password VARCHAR(40) NOT NULL,
  status char(1) default NULL,
  PRIMARY KEY  (user_id)
);

COMMENT ON TABLE users_table IS 'Utilisateurs';
COMMENT ON COLUMN users_table.user_id IS 'Identifiant unique de l''utilisateur';
COMMENT ON COLUMN users_table.username IS 'Nom de l''utilisateur';
COMMENT ON COLUMN users_table.group_id IS 'Identifiant unique du groupe d''appartenance de l''utilisateur';
COMMENT ON COLUMN users_table.email IS 'Adresse de courrier electronique de l''utilisateur';
COMMENT ON COLUMN users_table.password IS 'Mot de passe';
COMMENT ON COLUMN users_table.status IS 'Status (V=En cours de validation, C=Cree)';

CREATE VIEW users AS SELECT * from users_table;

GRANT SELECT, UPDATE ON users_user_id_seq TO admin_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE users_table TO admin_role;
GRANT SELECT, UPDATE ON TABLE users_table TO writer_role, reader_role;
GRANT SELECT ON users TO admin_role, writer_role, reader_role;

-- Utilisateur admin
INSERT INTO users_table
	(user_id,username,group_id,email,password,status)
VALUES
 (1,'dba',NULL,'','','C');


-- ********************************************
-- GROUPES 
-- ********************************************
CREATE SEQUENCE groups_group_id_seq;

CREATE TABLE groups_table (
  group_id bigint NOT NULL DEFAULT nextval('groups_group_id_seq'),
  group_name varchar(64) NOT NULL,
  owner_id bigint NOT NULL,
  parent_id bigint default NULL,
  PRIMARY KEY  (group_id)
);

COMMENT ON TABLE groups_table IS 'Groupes';
COMMENT ON COLUMN groups_table.group_id IS 'Identifiant unique du groupe';
COMMENT ON COLUMN groups_table.group_name IS 'Nom du groupe';
COMMENT ON COLUMN groups_table.owner_id IS 'Identifiant unique de l''utilisateur proprietaire du groupe';
COMMENT ON COLUMN groups_table.parent_id IS 'Identifiant unique du groupe parent';

CREATE VIEW groups AS SELECT * from groups_table;

ALTER TABLE users_table ADD FOREIGN KEY (group_id) REFERENCES groups_table (group_id) ON DELETE CASCADE;

ALTER TABLE groups_table ADD FOREIGN KEY (owner_id) REFERENCES users_table (user_id) ON DELETE CASCADE;
ALTER TABLE groups_table ADD FOREIGN KEY (parent_id) REFERENCES groups_table (group_id) ON DELETE CASCADE;

GRANT SELECT, UPDATE ON groups_group_id_seq TO admin_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE groups_table TO admin_role;
GRANT SELECT ON TABLE groups_table TO writer_role, reader_role;
GRANT SELECT ON groups TO admin_role, writer_role, reader_role;

-- ********************************************
-- UTILISATEURS dans les GROUPES
-- ********************************************
CREATE TABLE users_groups_table (
  user_id bigint NOT NULL,
  group_id bigint NOT NULL,
  PRIMARY KEY  (user_id,group_id),
  FOREIGN KEY (user_id) REFERENCES users_table(user_id) ON DELETE CASCADE,
  FOREIGN KEY (group_id) REFERENCES groups_table(group_id) ON DELETE CASCADE
);

COMMENT ON TABLE users_groups_table IS 'Associations utilisateur/groupe';
COMMENT ON COLUMN users_groups_table.user_id IS 'Identifiant unique de l''utilisateur';
COMMENT ON COLUMN users_groups_table.group_id IS 'Identifiant unique du groupe';

CREATE VIEW users_groups AS select * from users_groups_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE users_groups_table TO admin_role;
GRANT SELECT ON TABLE users_groups_table TO writer_role, reader_role;
GRANT SELECT ON users_groups TO admin_role, writer_role, reader_role;

-- ********************************************
-- PROJETS
-- ********************************************
CREATE SEQUENCE projects_project_id_seq;

CREATE TABLE projects_table (
  project_id bigint NOT NULL DEFAULT nextval('projects_project_id_seq'),
  owner_id bigint,
  short_name VARCHAR(128),
  description VARCHAR(16384),
  description_plain_text VARCHAR(16384),
  PRIMARY KEY (project_id),
  FOREIGN KEY (owner_id) REFERENCES users_table(user_id) ON DELETE SET NULL
);

COMMENT ON TABLE projects_table IS 'Projets';
COMMENT ON COLUMN projects_table.project_id IS 'Identifiant unique du projet';
COMMENT ON COLUMN projects_table.owner_id IS 'Identifiant unique de l''utilisateur proprietaire';
COMMENT ON COLUMN projects_table.short_name IS 'Nom abrege du projet';
COMMENT ON COLUMN projects_table.description IS 'Description du projet';
COMMENT ON COLUMN projects_table.description_plain_text IS 'Description du projet (texte brut)';

CREATE VIEW projects AS select * from projects_table;

GRANT SELECT, UPDATE ON projects_project_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE projects_table TO admin_role, writer_role;
GRANT SELECT ON TABLE projects_table TO reader_role;
GRANT SELECT ON projects TO admin_role, writer_role, reader_role;

CREATE OR REPLACE FUNCTION PROJECTS_TABLE_TRIGGER_FUNCTION()
RETURNS trigger AS $$
BEGIN
	NEW.description_plain_text:=html_to_plain_text(NEW.description);
	return NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER PROJECTS_TABLE_TRIGGER BEFORE INSERT OR UPDATE
ON projects_table  FOR EACH ROW
EXECUTE PROCEDURE PROJECTS_TABLE_TRIGGER_FUNCTION();

-- ********************************************
-- PARAMETRES DE PROJETS
-- ********************************************
CREATE SEQUENCE projects_parameters_project_parameter_id_seq;

CREATE TABLE projects_parameters_table (
  project_parameter_id bigint NOT NULL DEFAULT nextval('projects_parameters_project_parameter_id_seq'),
  project_id bigint NOT NULL,
  parameter_name VARCHAR(256) NOT NULL,
  parameter_value VARCHAR(256),
  PRIMARY KEY (project_parameter_id),
  FOREIGN KEY (project_id) REFERENCES projects_table(project_id) ON DELETE CASCADE
);

COMMENT ON TABLE projects_parameters_table IS 'Paramètres de projets';
COMMENT ON COLUMN projects_parameters_table.project_parameter_id IS 'Identifiant unique du paramètre';
COMMENT ON COLUMN projects_parameters_table.project_id IS 'Identifiant du projet associé';
COMMENT ON COLUMN projects_parameters_table.parameter_name IS 'Nom du paramètre';
COMMENT ON COLUMN projects_parameters_table.parameter_value IS 'Valeur du paramètre';

CREATE VIEW projects_parameters AS select * from projects_parameters_table;

GRANT SELECT, UPDATE ON projects_parameters_project_parameter_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE projects_parameters_table TO admin_role, writer_role;
GRANT SELECT ON TABLE projects_parameters_table TO reader_role;
GRANT SELECT ON projects_parameters TO admin_role, writer_role, reader_role;


-- ********************************************
-- VERSIONS DE PROJETS
-- ********************************************
CREATE SEQUENCE projects_versions_project_version_id_seq;

CREATE TABLE projects_versions_table (
	project_version_id BIGINT NOT NULL DEFAULT nextval('projects_versions_project_version_id_seq'),
	project_id BIGINT NOT NULL,
	version VARCHAR(12) NOT NULL DEFAULT '0.0.0.0',
	description VARCHAR(16384),
	description_plain_text VARCHAR(16384),
	bug_tracker_type VARCHAR(64),	
	bug_tracker_host VARCHAR(64),
	bug_tracker_url VARCHAR(256),
	bug_tracker_project_id VARCHAR(64),
	bug_tracker_project_version VARCHAR(64),
	PRIMARY KEY(project_id, version),
	UNIQUE(project_version_id),
	FOREIGN KEY(project_id) REFERENCES projects_table(project_id) ON DELETE CASCADE
);

COMMENT ON TABLE projects_versions_table IS 'Versions de projets';
COMMENT ON COLUMN projects_versions_table.project_version_id IS 'Identifiant unique de la version du projet';
COMMENT ON COLUMN projects_versions_table.project_id IS 'Identifiant du projet';
COMMENT ON COLUMN projects_versions_table.version IS 'Numéro de version';
COMMENT ON COLUMN projects_versions_table.description IS 'Description';
COMMENT ON COLUMN projects_versions_table.description_plain_text IS 'Description de la version du projet (texte brut)';
COMMENT ON COLUMN projects_versions_table.bug_tracker_type IS 'Type de bugtracker';
COMMENT ON COLUMN projects_versions_table.bug_tracker_host IS 'Hôte du bugtracker';
COMMENT ON COLUMN projects_versions_table.bug_tracker_url IS 'Url du bugtracker';
COMMENT ON COLUMN projects_versions_table.bug_tracker_project_id IS 'Identifiant du projet dans le bugtracker';
COMMENT ON COLUMN projects_versions_table.bug_tracker_project_version IS 'Identifiant de la version du projet dans le bugtracker';

CREATE VIEW projects_versions AS SELECT * FROM projects_versions_table;

GRANT SELECT, UPDATE ON projects_versions_project_version_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE projects_versions_table TO admin_role, writer_role;
GRANT SELECT ON TABLE projects_versions_table TO reader_role;
GRANT SELECT ON projects_versions TO admin_role, writer_role, reader_role;

CREATE OR REPLACE FUNCTION PROJECTS_VERSIONS_TABLE_TRIGGER_FUNCTION()
RETURNS trigger AS $$
BEGIN
	NEW.description_plain_text:=html_to_plain_text(NEW.description);
	return NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER PROJECTS_VERSIONS_TABLE_TRIGGER BEFORE INSERT OR UPDATE
ON projects_versions_table  FOR EACH ROW
EXECUTE PROCEDURE PROJECTS_VERSIONS_TABLE_TRIGGER_FUNCTION();

-- ********************************************
-- CONTENUS DE TESTS
-- ********************************************
CREATE SEQUENCE tests_contents_test_content_id_seq;
CREATE SEQUENCE tests_contents_original_test_content_id_seq;

CREATE TABLE tests_contents_table (
	test_content_id BIGINT DEFAULT nextval('tests_contents_test_content_id_seq'),
	original_test_content_id BIGINT NOT NULL DEFAULT nextval('tests_contents_original_test_content_id_seq'),
	short_name VARCHAR(128),
	description VARCHAR(16384),
  	description_plain_text VARCHAR(16384),
	category_id CHAR(1) DEFAULT 'F',
	priority_level INTEGER DEFAULT 3,
	version VARCHAR(12) NOT NULL,
	project_id BIGINT NOT NULL,
	status CHAR(1) DEFAULT 'I',
	automated CHAR(1) DEFAULT 'N',
	automation_command VARCHAR(512),
	automation_command_parameters VARCHAR(512),
	type CHAR(1) DEFAULT 'N',
	limit_test_case CHAR(1) DEFAULT 'N',
	PRIMARY KEY(test_content_id),
	FOREIGN KEY (project_id, version) REFERENCES projects_versions_table (project_id, version) ON UPDATE CASCADE ON DELETE CASCADE
);
ALTER TABLE tests_contents_table ADD CONSTRAINT original_test_content_version_pk UNIQUE (original_test_content_id, version);

COMMENT ON TABLE tests_contents_table IS 'Contenus des tests';
COMMENT ON COLUMN tests_contents_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN tests_contents_table.original_test_content_id IS 'Identifiant du contenu de test original';
COMMENT ON COLUMN tests_contents_table.short_name IS 'Nom court du test';
COMMENT ON COLUMN tests_contents_table.description IS 'Description du test';
COMMENT ON COLUMN tests_contents_table.priority_level IS 'Priorité du test';
COMMENT ON COLUMN tests_contents_table.version IS 'Version du projet';
COMMENT ON COLUMN tests_contents_table.project_id IS 'Identifiant du projet';
COMMENT ON COLUMN tests_contents_table.category_id IS 'Catégorie du test';
COMMENT ON COLUMN tests_contents_table.description_plain_text IS 'Description de l''exigence (texte brut)';
COMMENT ON COLUMN tests_contents_table.status IS 'Status du test';
COMMENT ON COLUMN tests_contents_table.automated IS 'Test automatise (N=No, Y=Yes)';
COMMENT ON COLUMN tests_contents_table.automation_command IS 'Ligne de commande utilisee pour l''automatisation';
COMMENT ON COLUMN tests_contents_table.automation_command_parameters IS 'Parametres de la ligne de commande utilisee pour l''automatisation';
COMMENT ON COLUMN tests_contents_table.type IS 'Type du test (N=Nominal, A=Alternatif, E=Exception)';
COMMENT ON COLUMN tests_contents_table.limit_test_case IS 'Cas aux limites (N=No, Y=Yes)';

CREATE VIEW tests_contents AS SELECT * FROM tests_contents_table;

GRANT SELECT, UPDATE ON tests_contents_test_content_id_seq TO admin_role, writer_role;
GRANT SELECT, UPDATE ON tests_contents_original_test_content_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_contents_table TO admin_role, writer_role;
GRANT SELECT ON TABLE tests_contents_table TO reader_role;
GRANT SELECT ON tests_contents TO admin_role, writer_role, reader_role;

CREATE OR REPLACE FUNCTION TESTS_CONTENTS_TABLE_TRIGGER_FUNCTION()
RETURNS trigger AS $$
BEGIN
	NEW.description_plain_text:=html_to_plain_text(NEW.description);
	return NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER TESTS_CONTENTS_TABLE_TRIGGER BEFORE INSERT OR UPDATE
ON tests_contents_table  FOR EACH ROW
EXECUTE PROCEDURE TESTS_CONTENTS_TABLE_TRIGGER_FUNCTION();

-- ********************************************
-- TESTS
-- ********************************************
CREATE SEQUENCE tests_test_id_seq;

CREATE TABLE tests_table (
	test_id bigint NOT NULL DEFAULT nextval('tests_test_id_seq'),
	test_content_id BIGINT NOT NULL,
	original_test_id bigint,
	parent_test_id bigint,
	previous_test_id bigint,
	project_id bigint NOT NULL,
	version VARCHAR(12) NOT NULL,
	PRIMARY KEY (test_id),
	FOREIGN KEY (test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE,
	FOREIGN KEY (original_test_id) REFERENCES tests_table(test_id) ON DELETE SET NULL,
	FOREIGN KEY (parent_test_id) REFERENCES tests_table(test_id) ON DELETE CASCADE,
	FOREIGN KEY (previous_test_id) REFERENCES tests_table(test_id) ON DELETE SET NULL,
	FOREIGN KEY (project_id,version) REFERENCES projects_versions_table(project_id,version) ON UPDATE CASCADE ON DELETE CASCADE
);

COMMENT ON TABLE tests_table IS 'Scénarios(Tests)';
COMMENT ON COLUMN tests_table.test_id IS 'Identifiant unique du test';
COMMENT ON COLUMN tests_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN tests_table.original_test_id IS 'Identifiant du test original';
COMMENT ON COLUMN tests_table.parent_test_id IS 'Identifiant du test parent';
COMMENT ON COLUMN tests_table.previous_test_id IS 'Identifiant du test precedent';
COMMENT ON COLUMN tests_table.project_id IS 'Identifiant du projet associe';
COMMENT ON COLUMN tests_table.version IS 'Numéro de version';

CREATE VIEW tests AS select * from tests_table;

GRANT SELECT, UPDATE ON tests_test_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_table TO admin_role, writer_role;
GRANT SELECT ON TABLE tests_table TO reader_role;
GRANT SELECT ON tests TO admin_role, writer_role, reader_role;
CREATE OR REPLACE VIEW tests_contents AS select * from tests_contents_table;
GRANT SELECT ON tests_contents TO admin_role, writer_role, reader_role;

CREATE OR REPLACE VIEW TESTS_HIERARCHY AS 
SELECT
	tests_contents_table.short_name AS short_name,
	tests_contents_table.category_id AS category_id,
	tests_contents_table.priority_level AS priority_level,
	tests_contents_table.version AS content_version,
	tests_table.test_id AS test_id,
	tests_table.test_content_id AS test_content_id,
	tests_table.original_test_id AS original_test_id,
	tests_table.parent_test_id AS parent_test_id,
	tests_table.previous_test_id AS previous_test_id,
	tests_table.project_id AS project_id,
	tests_table.version AS version,
	tests_contents_table.status AS status,
	tests_contents_table.original_test_content_id AS original_test_content_id,
	tests_contents_table.automated AS content_automated,
	tests_contents_table.type AS content_type
FROM 
	tests_contents_table, tests_table 
WHERE
	tests_contents_table.test_content_id = tests_table.test_content_id;
GRANT SELECT ON TESTS_HIERARCHY TO admin_role, writer_role, reader_role;

-- ********************************************
-- ACTIONS
-- ********************************************
CREATE SEQUENCE actions_action_id_seq;

CREATE TABLE actions_table (
  action_id bigint NOT NULL DEFAULT nextval('actions_action_id_seq'),
  previous_action_id bigint,
  test_content_id bigint NOT NULL,
  short_name VARCHAR(128),
  description VARCHAR(16384),
  description_plain_text VARCHAR(16384),
  wait_result VARCHAR(16384),
  wait_result_plain_text VARCHAR(16384),
  link_original_test_content_id bigint,
  PRIMARY KEY (action_id),
  FOREIGN KEY (previous_action_id) REFERENCES actions_table(action_id) ON DELETE SET NULL,
  FOREIGN KEY (test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE
);

COMMENT ON TABLE actions_table IS 'Actions';
COMMENT ON COLUMN actions_table.action_id IS 'Identifiant unique de l''action';
COMMENT ON COLUMN actions_table.previous_action_id IS 'Identifiant de l''action precedente';
COMMENT ON COLUMN actions_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN actions_table.short_name IS 'Nom abrege de l''action';
COMMENT ON COLUMN actions_table.description IS 'Description de l''action';
COMMENT ON COLUMN actions_table.wait_result IS 'Resultat attendu';
COMMENT ON COLUMN actions_table.description_plain_text IS 'Description de l''action (texte brut)';
COMMENT ON COLUMN actions_table.wait_result_plain_text IS 'Resultat attendu (texte brut)';
COMMENT ON COLUMN actions_table.link_original_test_content_id IS 'Identifiant du test original lié';

CREATE VIEW actions AS select * from actions_table;

GRANT SELECT, UPDATE ON actions_action_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE actions_table TO admin_role, writer_role;
GRANT SELECT ON TABLE actions_table TO reader_role;
GRANT SELECT ON actions TO admin_role, writer_role, reader_role;

CREATE OR REPLACE FUNCTION ACTIONS_TABLE_TRIGGER_FUNCTION()
RETURNS trigger AS $$
BEGIN
	NEW.description_plain_text:=html_to_plain_text(NEW.description);
	NEW.wait_result_plain_text:=html_to_plain_text(NEW.wait_result);
	return NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER ACTIONS_TABLE_TRIGGER BEFORE INSERT OR UPDATE
ON actions_table  FOR EACH ROW
EXECUTE PROCEDURE ACTIONS_TABLE_TRIGGER_FUNCTION();


-- ********************************************
-- ACTIONS AUTOMATISEES
-- ********************************************
CREATE SEQUENCE automated_actions_action_id_seq;

CREATE TABLE automated_actions_table (
  automated_action_id bigint NOT NULL DEFAULT nextval('automated_actions_action_id_seq'),
  previous_automated_action_id bigint,
  test_content_id bigint NOT NULL,
  window_id VARCHAR(1024),
  message_type int,
  message_data VARCHAR(1024),
  message_time_delay int,
  PRIMARY KEY (automated_action_id),
  FOREIGN KEY (previous_automated_action_id) REFERENCES automated_actions_table(automated_action_id) ON DELETE SET NULL,
  FOREIGN KEY (test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE
);

COMMENT ON TABLE automated_actions_table IS 'Actions';
COMMENT ON COLUMN automated_actions_table.automated_action_id IS 'Identifiant unique de l''action';
COMMENT ON COLUMN automated_actions_table.previous_automated_action_id IS 'Identifiant de l''action precedente';
COMMENT ON COLUMN automated_actions_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN automated_actions_table.window_id IS 'Identifiant de la fenetre';
COMMENT ON COLUMN automated_actions_table.message_type IS 'Type de message envoye a la fenetre';
COMMENT ON COLUMN automated_actions_table.message_data IS 'Contenu du message envoye a la fenetre';
COMMENT ON COLUMN automated_actions_table.message_time_delay IS 'Delai avant traitement du message suivant';

CREATE OR REPLACE VIEW automated_actions AS select * from automated_actions_table;

GRANT SELECT, UPDATE ON automated_actions_action_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE automated_actions_table TO admin_role, writer_role;
GRANT SELECT ON TABLE automated_actions_table TO reader_role;
GRANT SELECT ON automated_actions TO admin_role, writer_role, reader_role;

-- ********************************************
-- VALIDATIONS DES ACTIONS AUTOMATISEES
-- ********************************************
CREATE SEQUENCE automated_actions_validations_validation_id_seq;

CREATE TABLE automated_actions_validations_table (
  validation_id bigint NOT NULL DEFAULT nextval('automated_actions_validations_validation_id_seq'),
  previous_validation_id bigint,
  automated_action_id bigint NOT NULL,
  module_name VARCHAR(128),
  module_version VARCHAR(64),
  module_function_name VARCHAR(128),
  module_function_parameters VARCHAR(256),  
  PRIMARY KEY (validation_id),
  FOREIGN KEY (previous_validation_id) REFERENCES automated_actions_validations_table(validation_id) ON DELETE SET NULL,
  FOREIGN KEY (automated_action_id) REFERENCES automated_actions_table(automated_action_id) ON DELETE CASCADE
);

COMMENT ON TABLE automated_actions_validations_table IS 'Validations des actions automatisees';
COMMENT ON COLUMN automated_actions_validations_table.validation_id IS 'Identifiant unique de la validation';
COMMENT ON COLUMN automated_actions_validations_table.previous_validation_id IS 'Identifiant de la validation precedente';
COMMENT ON COLUMN automated_actions_validations_table.automated_action_id IS 'Identifiant de l''action automatisee associee';
COMMENT ON COLUMN automated_actions_validations_table.module_name IS 'Nom du module externe';
COMMENT ON COLUMN automated_actions_validations_table.module_version IS 'Version du module externe';
COMMENT ON COLUMN automated_actions_validations_table.module_function_name IS 'Nom de la fonction du module externe';
COMMENT ON COLUMN automated_actions_validations_table.module_function_parameters IS 'Liste des paramètres de la fonction du module externe';


CREATE OR REPLACE VIEW automated_actions_validations AS select * from automated_actions_validations_table;

GRANT SELECT, UPDATE ON automated_actions_validations_validation_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE automated_actions_validations_table TO admin_role, writer_role;
GRANT SELECT ON TABLE automated_actions_validations_table TO reader_role;
GRANT SELECT ON automated_actions_validations TO admin_role, writer_role, reader_role;


-- ********************************************
-- CONTENUS D'EXIGENCES
-- ********************************************
CREATE SEQUENCE requirements_contents_requirement_content_id_seq;
CREATE SEQUENCE requirements_contents_original_requirement_content_id_seq;

CREATE TABLE requirements_contents_table (
	requirement_content_id bigint DEFAULT nextval('requirements_contents_requirement_content_id_seq'),
	original_requirement_content_id bigint NOT NULL DEFAULT nextval('requirements_contents_original_requirement_content_id_seq'),
	project_id bigint NOT NULL,
	version VARCHAR(12) NOT NULL,
	short_name VARCHAR(128),
	description VARCHAR(16384),
	description_plain_text VARCHAR(16384),
	category_id CHAR(1) DEFAULT 'F',
	priority_level INTEGER DEFAULT 3,
	status CHAR(1) DEFAULT 'I',
	PRIMARY KEY (requirement_content_id),
	FOREIGN KEY (project_id,version) REFERENCES projects_versions_table(project_id,version) ON UPDATE CASCADE ON DELETE CASCADE
);
ALTER TABLE requirements_contents_table ADD CONSTRAINT original_requirement_content_version_pk UNIQUE (original_requirement_content_id, version);

COMMENT ON TABLE requirements_contents_table IS 'Contenus des exigences';
COMMENT ON COLUMN requirements_contents_table.requirement_content_id  IS 'Identifiant unique du contenu d''exigence';
COMMENT ON COLUMN requirements_contents_table.project_id IS 'Identifiant du projet associe';
COMMENT ON COLUMN requirements_contents_table.version IS 'Numéro de version';
COMMENT ON COLUMN requirements_contents_table.short_name IS 'Nom abrege de l''exigence';
COMMENT ON COLUMN requirements_contents_table.description IS 'Description de l''exigence';
COMMENT ON COLUMN requirements_contents_table.category_id IS 'Identifiant de la categorie d''exigence';
COMMENT ON COLUMN requirements_contents_table.priority_level IS 'Priorité de l''exigence';
COMMENT ON COLUMN requirements_contents_table.original_requirement_content_id  IS 'Identifiant du contenu d''exigence original';
COMMENT ON COLUMN requirements_contents_table.description_plain_text IS 'Description de l''exigence (texte brut)';
COMMENT ON COLUMN requirements_contents_table.status IS 'Status de l''exigence';

CREATE VIEW requirements_contents AS select * from requirements_contents_table;

GRANT SELECT, UPDATE ON requirements_contents_requirement_content_id_seq TO admin_role, writer_role;
GRANT SELECT, UPDATE ON requirements_contents_original_requirement_content_id_seq TO admin_role, writer_role;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE requirements_contents_table TO admin_role, writer_role;
GRANT SELECT ON TABLE requirements_contents_table TO reader_role;
GRANT SELECT ON requirements_contents TO admin_role, writer_role, reader_role;

CREATE OR REPLACE FUNCTION REQUIREMENTS_CONTENTS_TABLE_TRIGGER_FUNCTION()
RETURNS trigger AS $$
BEGIN
	NEW.description_plain_text:=html_to_plain_text(NEW.description);
	return NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER REQUIREMENTS_CONTENTS_TABLE_TRIGGER BEFORE INSERT OR UPDATE
ON requirements_contents_table  FOR EACH ROW
EXECUTE PROCEDURE REQUIREMENTS_CONTENTS_TABLE_TRIGGER_FUNCTION();

-- ********************************************
-- EXIGENCES
-- ********************************************
CREATE SEQUENCE requirements_requirement_id_seq;

CREATE TABLE requirements_table (
	requirement_id bigint NOT NULL DEFAULT nextval('requirements_requirement_id_seq'),
	requirement_content_id bigint NOT NULL,
	parent_requirement_id bigint,
	previous_requirement_id bigint,
	project_id bigint NOT NULL,
	version VARCHAR(12) NOT NULL,
	PRIMARY KEY (requirement_id),
	FOREIGN KEY (requirement_content_id) REFERENCES requirements_contents_table(requirement_content_id) ON DELETE CASCADE,
	FOREIGN KEY (parent_requirement_id) REFERENCES requirements_table(requirement_id) ON DELETE CASCADE,
	FOREIGN KEY (previous_requirement_id) REFERENCES requirements_table(requirement_id) ON DELETE SET NULL,
	FOREIGN KEY (project_id,version) REFERENCES projects_versions_table(project_id,version) ON UPDATE CASCADE ON DELETE CASCADE
);

COMMENT ON TABLE requirements_table IS 'Exigences';
COMMENT ON COLUMN requirements_table.requirement_id IS 'Identifiant unique de l''exigence';
COMMENT ON COLUMN requirements_table.requirement_content_id  IS 'Identifiant du contenu d''exigence';
COMMENT ON COLUMN requirements_table.parent_requirement_id IS 'Identifiant de l''exigence parent';
COMMENT ON COLUMN requirements_table.previous_requirement_id IS 'Identifiant de l''exigence precedente';
COMMENT ON COLUMN requirements_table.project_id IS 'Identifiant du projet associe';
COMMENT ON COLUMN requirements_table.version IS 'Numéro de version';

CREATE VIEW requirements AS select * from requirements_table;

GRANT SELECT, UPDATE ON requirements_requirement_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE requirements_table TO admin_role, writer_role;
GRANT SELECT ON TABLE requirements_table TO reader_role;
GRANT SELECT ON requirements TO admin_role, writer_role, reader_role;

CREATE OR REPLACE VIEW REQUIREMENTS_HIERARCHY AS 
SELECT
	requirements_contents_table.short_name AS short_name,
	requirements_contents_table.category_id AS category_id,
	requirements_contents_table.priority_level AS priority_level,
	requirements_contents_table.version AS content_version,
	requirements_table.requirement_id AS requirement_id,
	requirements_table.requirement_content_id AS requirement_content_id,
	requirements_table.parent_requirement_id AS parent_requirement_id,
	requirements_table.previous_requirement_id AS previous_requirement_id,
	requirements_table.project_id AS project_id,
	requirements_table.version AS version,
	requirements_contents_table.status AS status,
	requirements_contents_table.original_requirement_content_id AS original_requirement_content_id 
FROM 
	requirements_contents_table, requirements_table 
WHERE
	requirements_contents_table.requirement_content_id = requirements_table.requirement_content_id;
GRANT SELECT ON REQUIREMENTS_HIERARCHY TO admin_role, writer_role, reader_role;


-- ********************************************
-- CATEGORIES D'EXIGENCES
-- ********************************************
CREATE TABLE requirements_categories_table (
	category_id CHAR(1) PRIMARY KEY,
	category_label VARCHAR(64) NOT NULL
);

COMMENT ON TABLE requirements_categories_table IS 'Categories d''exigences';
COMMENT ON COLUMN requirements_categories_table.category_id IS 'Identifiant de la categorie d''exigence';
COMMENT ON COLUMN requirements_categories_table.category_label IS 'Libelle de la categorie';

CREATE OR REPLACE VIEW requirements_categories AS SELECT * FROM requirements_categories_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE requirements_categories_table TO admin_role;
GRANT SELECT ON requirements_categories_table TO writer_role, reader_role;
GRANT SELECT ON requirements_categories TO admin_role, writer_role, reader_role;

INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('F', 'Fonctionnel');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('I', 'Interopérabilité');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('C', 'Charge');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('P', 'Performance');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('D', 'Disponibilité');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('S', 'Sécurité');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('E', 'Exploitabilité');
INSERT INTO requirements_categories_table (category_id, category_label)
VALUES ('Z', 'Autre');

-- ********************************************
-- EXIGENCES DE TESTS
-- ********************************************
CREATE TABLE tests_requirements_table (
	test_content_id bigint NOT NULL,
	original_requirement_content_id bigint NOT NULL,
	PRIMARY KEY (test_content_id, original_requirement_content_id),
	FOREIGN KEY(test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE
);

COMMENT ON TABLE tests_requirements_table IS 'Associations exigences/tests';
COMMENT ON COLUMN tests_requirements_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN tests_requirements_table.original_requirement_content_id IS 'Identifiant original du contenu de l''exigence';

CREATE VIEW tests_requirements AS SELECT * FROM tests_requirements_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_requirements_table TO admin_role, writer_role;
GRANT SELECT ON TABLE tests_requirements_table TO reader_role;
GRANT SELECT ON tests_requirements TO admin_role, writer_role, reader_role;

-- ********************************************
-- CAMPAGNES
-- ********************************************
CREATE SEQUENCE campaigns_campaign_id_seq;

CREATE TABLE campaigns_table (
	campaign_id bigint PRIMARY KEY DEFAULT nextval('campaigns_campaign_id_seq'),
	project_id bigint NOT NULL,
	version VARCHAR(12) NOT NULL,
	short_name VARCHAR(128),
	description VARCHAR(16384),
	FOREIGN KEY (project_id, version) REFERENCES projects_versions_table (project_id, version) ON UPDATE CASCADE ON DELETE CASCADE
);

COMMENT ON TABLE campaigns_table IS 'Campagnes de tests';
COMMENT ON COLUMN campaigns_table.campaign_id IS 'Identifiant unique de la campagne';
COMMENT ON COLUMN campaigns_table.project_id IS 'Identifiant du projet associe';
COMMENT ON COLUMN campaigns_table.version IS 'Version du projet associe';
COMMENT ON COLUMN campaigns_table.short_name IS 'Nom abrege de la campagne';
COMMENT ON COLUMN campaigns_table.description IS 'Description de la campagne';

CREATE VIEW campaigns AS select * from campaigns_table;

GRANT SELECT, UPDATE ON campaigns_campaign_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE campaigns_table TO admin_role, writer_role;
GRANT SELECT ON TABLE campaigns_table TO reader_role;
GRANT SELECT ON campaigns TO admin_role, writer_role, reader_role;

-- ********************************************
-- TESTS DE CAMPAGNES
-- ********************************************
CREATE SEQUENCE tests_campaigns_test_campaign_id_seq;

CREATE TABLE tests_campaigns_table (
	test_campaign_id bigint PRIMARY KEY DEFAULT nextval('tests_campaigns_test_campaign_id_seq'),
	campaign_id bigint NOT NULL,
	test_id bigint NOT NULL,
	previous_test_campaign_id bigint,
	FOREIGN KEY (campaign_id) REFERENCES campaigns_table (campaign_id) ON DELETE CASCADE,
	FOREIGN KEY (test_id) REFERENCES tests_table (test_id) ON DELETE CASCADE
);
ALTER TABLE tests_campaigns_table
ADD FOREIGN KEY (previous_test_campaign_id) REFERENCES tests_campaigns_table (test_campaign_id) ON DELETE SET NULL;

COMMENT ON TABLE tests_campaigns_table IS 'Tests de campagnes';
COMMENT ON COLUMN tests_campaigns_table.test_campaign_id IS 'Identifiant du test de campagne';
COMMENT ON COLUMN tests_campaigns_table.campaign_id IS 'Identifiant de la campagne';
COMMENT ON COLUMN tests_campaigns_table.test_id IS 'Identifiant du test dans le projet';
COMMENT ON COLUMN tests_campaigns_table.previous_test_campaign_id IS 'Identifiant du test précédent dans la campagne';

CREATE VIEW tests_campaigns AS select * from tests_campaigns_table;

GRANT SELECT, UPDATE ON tests_campaigns_test_campaign_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_campaigns_table TO admin_role, writer_role;
GRANT SELECT ON TABLE tests_campaigns_table TO reader_role;
GRANT SELECT ON tests_campaigns TO admin_role, writer_role, reader_role;

-- ********************************************
-- EXECUTIONS DE CAMPAGNES
-- ********************************************
CREATE SEQUENCE executions_campaigns_execution_campaign_id_seq;

CREATE TABLE executions_campaigns_table (
	execution_campaign_id bigint PRIMARY KEY DEFAULT nextval('executions_campaigns_execution_campaign_id_seq'),
	campaign_id bigint NOT NULL,
	execution_date timestamp with time zone NOT NULL DEFAULT now(),
	revision VARCHAR(24),
	user_id bigint,
	FOREIGN KEY (campaign_id) REFERENCES campaigns_table (campaign_id) ON DELETE CASCADE,
	FOREIGN KEY (user_id) REFERENCES users_table (user_id) ON DELETE SET NULL
);

COMMENT ON TABLE executions_campaigns_table IS 'Exécutions de campagnes';
COMMENT ON COLUMN executions_campaigns_table.execution_campaign_id IS 'Identifiant unique de l''exécution de campagne';
COMMENT ON COLUMN executions_campaigns_table.campaign_id IS 'Identifiant de la campagne';
COMMENT ON COLUMN executions_campaigns_table.execution_date IS 'Date de l''exécution de la campagne';
COMMENT ON COLUMN executions_campaigns_table.revision IS 'Revision';
COMMENT ON COLUMN executions_campaigns_table.user_id IS 'Identifiant de l''utilisateur';


CREATE VIEW executions_campaigns AS select * from executions_campaigns_table;

GRANT SELECT, UPDATE ON executions_campaigns_execution_campaign_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_campaigns_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_campaigns_table TO reader_role;
GRANT SELECT ON executions_campaigns TO admin_role, writer_role, reader_role;

-- ********************************************
-- PARAMETRES D'EXECUTIONS DE CAMPAGNES
-- ********************************************
CREATE SEQUENCE executions_campaigns_parameters_parameter_id_seq;

CREATE TABLE executions_campaigns_parameters_table (
  execution_campaign_parameter_id bigint NOT NULL DEFAULT nextval('executions_campaigns_parameters_parameter_id_seq'),
  execution_campaign_id bigint NOT NULL,
  parameter_name VARCHAR(256) NOT NULL,
  parameter_value VARCHAR(256),
  PRIMARY KEY (execution_campaign_parameter_id),
  FOREIGN KEY (execution_campaign_id) REFERENCES executions_campaigns_table(execution_campaign_id) ON DELETE CASCADE
);

COMMENT ON TABLE executions_campaigns_parameters_table IS 'Paramètres d''exécutions de campagnes';
COMMENT ON COLUMN executions_campaigns_parameters_table.execution_campaign_parameter_id IS 'Identifiant unique du paramètre';
COMMENT ON COLUMN executions_campaigns_parameters_table.execution_campaign_id IS 'Identifiant de l''exécution de campagne associée';
COMMENT ON COLUMN executions_campaigns_parameters_table.parameter_name IS 'Nom du paramètre';
COMMENT ON COLUMN executions_campaigns_parameters_table.parameter_value IS 'Valeur du paramètre';

CREATE VIEW executions_campaigns_parameters AS select * from executions_campaigns_parameters_table;

GRANT SELECT, UPDATE ON executions_campaigns_parameters_parameter_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_campaigns_parameters_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_campaigns_parameters_table TO reader_role;
GRANT SELECT ON executions_campaigns_parameters TO admin_role, writer_role, reader_role;

-- ********************************************
-- EXECUTIONS DE TESTS DE CAMPAGNES
-- ********************************************
CREATE SEQUENCE executions_tests_execution_test_id_seq;

CREATE TABLE executions_tests_table (
	execution_test_id bigint PRIMARY KEY DEFAULT nextval('executions_tests_execution_test_id_seq'),
	execution_campaign_id bigint NOT NULL,
	parent_execution_test_id bigint,
	previous_execution_test_id bigint,
	test_id bigint NOT NULL,
	execution_date timestamp with time zone NOT NULL DEFAULT now(),
	result_id CHAR(1) NOT NULL,
	comments VARCHAR(16384),
	FOREIGN KEY (execution_campaign_id) REFERENCES executions_campaigns_table(execution_campaign_id) ON DELETE CASCADE,
	FOREIGN KEY (test_id) REFERENCES tests_table(test_id) ON DELETE CASCADE,
	FOREIGN KEY (parent_execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE CASCADE,
	FOREIGN KEY (previous_execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE SET NULL
);

COMMENT ON TABLE executions_tests_table IS 'Exécutions de tests de campagnes';
COMMENT ON COLUMN executions_tests_table.execution_test_id IS 'Identifiant unuique de l''exécution du test';
COMMENT ON COLUMN executions_tests_table.execution_campaign_id IS 'Identifiant de l''exécution de campagne';
COMMENT ON COLUMN executions_tests_table.parent_execution_test_id IS 'Identifiant du test parent';
COMMENT ON COLUMN executions_tests_table.previous_execution_test_id IS 'Identifiant du test precedent';
COMMENT ON COLUMN executions_tests_table.test_id IS 'Identifiant du test';
COMMENT ON COLUMN executions_tests_table.execution_date IS 'Date d''exécution du test';
COMMENT ON COLUMN executions_tests_table.result_id IS 'Résultat';
COMMENT ON COLUMN executions_tests_table.comments IS 'Commentaires';

CREATE VIEW executions_tests AS select * from executions_tests_table;

GRANT SELECT, UPDATE ON executions_tests_execution_test_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_tests_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_tests_table TO reader_role;
GRANT SELECT ON executions_tests TO admin_role, writer_role, reader_role;

CREATE TABLE tests_results_table (
	result_id CHAR(1) PRIMARY KEY,
	description VARCHAR(256)
);

COMMENT ON TABLE tests_results_table IS 'Résultats d''exécution de tests';
COMMENT ON COLUMN tests_results_table.result_id IS 'Identifiant du résultat d''exécution';
COMMENT ON COLUMN tests_results_table.description IS 'Description du résultat';

CREATE VIEW tests_results AS select * from tests_results_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_results_table TO admin_role;
GRANT SELECT ON TABLE tests_results_table TO writer_role, reader_role;
GRANT SELECT ON tests_results TO admin_role, writer_role, reader_role;

INSERT INTO tests_results_table (result_id, description)
VALUES ('0', 'Résultat OK');
INSERT INTO tests_results_table (result_id, description)
VALUES ('1', 'Résultat KO');
INSERT INTO tests_results_table (result_id, description)
VALUES ('2', 'Test non exécuté');
INSERT INTO tests_results_table (result_id, description)
VALUES ('3', 'Execution incomplète du test');

-----------------------------------------
--	EXECUTIONS D'ACTIONS DE CAMPAGNES
-----------------------------------------
CREATE SEQUENCE executions_actions_execution_action_id_seq;

CREATE TABLE executions_actions_table (
	execution_action_id bigint PRIMARY KEY DEFAULT nextval('executions_actions_execution_action_id_seq'),
	execution_test_id bigint NOT NULL,
	action_id bigint NOT NULL,
	previous_execution_action_id bigint,
	result_id CHAR(1) NOT NULL,
	comments VARCHAR(16384),
	FOREIGN KEY (execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE CASCADE,
	FOREIGN KEY (action_id) REFERENCES actions_table(action_id) ON DELETE CASCADE,
	FOREIGN KEY (previous_execution_action_id) REFERENCES executions_actions_table(execution_action_id) ON DELETE SET NULL
);

COMMENT ON TABLE executions_actions_table IS 'Exécutions d''actions de tests';
COMMENT ON COLUMN executions_actions_table.execution_action_id IS 'Identifiant unique de l''exécution de l''action';
COMMENT ON COLUMN executions_actions_table.execution_test_id IS 'Identifiant de l''exécution de test';
COMMENT ON COLUMN executions_actions_table.action_id IS 'Identifiant de l''action';
COMMENT ON COLUMN executions_actions_table.previous_execution_action_id IS 'Identifiant de l''action précédente';
COMMENT ON COLUMN executions_actions_table.result_id IS 'Identifiant du résultat';
COMMENT ON COLUMN executions_actions_table.comments IS 'Commentaires';


CREATE VIEW executions_actions AS select * from executions_actions_table;

GRANT SELECT, UPDATE ON executions_actions_execution_action_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_actions_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_actions_table TO reader_role;
GRANT SELECT ON executions_actions TO admin_role, writer_role, reader_role;

CREATE TABLE actions_results_table (
	result_id CHAR(1) PRIMARY KEY,
	description VARCHAR(256)
);

COMMENT ON TABLE actions_results_table IS 'Résultats d''exécution d''actions';
COMMENT ON COLUMN actions_results_table.result_id IS 'Identifiant du résultat d''exécution';
COMMENT ON COLUMN actions_results_table.description IS 'Description du résultat';

CREATE VIEW actions_results AS select * from actions_results_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE actions_results_table TO admin_role;
GRANT SELECT ON TABLE actions_results_table TO writer_role, reader_role;
GRANT SELECT ON actions_results TO admin_role, writer_role, reader_role;

INSERT INTO actions_results_table (result_id, description)
VALUES ('0', 'Résultat OK');
INSERT INTO actions_results_table (result_id, description)
VALUES ('1', 'Résultat KO');
INSERT INTO actions_results_table (result_id, description)
VALUES ('2', 'Action non exécutée');


-----------------------------------------
--	EXECUTIONS D'ACTIONS AUTOMATISEES DE CAMPAGNES
-----------------------------------------
CREATE SEQUENCE automated_executions_actions_execution_action_id_seq;

CREATE TABLE automated_executions_actions_table (
	automated_execution_action_id bigint PRIMARY KEY DEFAULT nextval('automated_executions_actions_execution_action_id_seq'),
	execution_test_id bigint NOT NULL,
	automated_action_id bigint NOT NULL,
	previous_automated_execution_action_id bigint,
	result_id CHAR(1) NOT NULL,
	comments VARCHAR(16384),
	FOREIGN KEY (execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE CASCADE,
	FOREIGN KEY (automated_action_id) REFERENCES automated_actions_table(automated_action_id) ON DELETE CASCADE,
	FOREIGN KEY (previous_automated_execution_action_id) REFERENCES automated_executions_actions_table(automated_execution_action_id) ON DELETE SET NULL
);

COMMENT ON TABLE automated_executions_actions_table IS 'Exécutions d''actions de tests';
COMMENT ON COLUMN automated_executions_actions_table.automated_execution_action_id IS 'Identifiant unique de l''exécution de l''action';
COMMENT ON COLUMN automated_executions_actions_table.execution_test_id IS 'Identifiant de l''exécution de test';
COMMENT ON COLUMN automated_executions_actions_table.automated_action_id IS 'Identifiant de l''action';
COMMENT ON COLUMN automated_executions_actions_table.previous_automated_execution_action_id IS 'Identifiant de l''action précédente';
COMMENT ON COLUMN automated_executions_actions_table.result_id IS 'Identifiant du résultat';
COMMENT ON COLUMN automated_executions_actions_table.comments IS 'Commentaires';


CREATE OR REPLACE VIEW automated_executions_actions AS select * from automated_executions_actions_table;

GRANT SELECT, UPDATE ON automated_executions_actions_execution_action_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE automated_executions_actions_table TO admin_role, writer_role;
GRANT SELECT ON TABLE automated_executions_actions_table TO reader_role;
GRANT SELECT ON automated_executions_actions TO admin_role, writer_role, reader_role;

-----------------------------------------
--	ANOMALIES
-----------------------------------------
CREATE SEQUENCE bugs_bug_id_seq;

CREATE TABLE bugs_table (
	bug_id bigint PRIMARY KEY DEFAULT nextval('bugs_bug_id_seq'),
	execution_test_id bigint NOT NULL,
	execution_action_id bigint,
	creation_date timestamp with time zone NOT NULL DEFAULT now(),
	short_name varchar(128),
	priority varchar(64),
	severity varchar(64),
	reproducibility varchar(64),
	platform varchar(64),
	system varchar(64),
	description varchar(16384),
	bugtracker_bug_id varchar(64),
	status char(1) DEFAULT 'O',
	FOREIGN KEY (execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE CASCADE,
	FOREIGN KEY (execution_action_id) REFERENCES executions_actions_table(execution_action_id) ON DELETE CASCADE

);

COMMENT ON TABLE bugs_table IS 'Anomalies';
COMMENT ON COLUMN bugs_table.bug_id IS 'Identifiant unique de l''anomalie';
COMMENT ON COLUMN bugs_table.execution_test_id IS 'Identifiant unique de l''exécution de test associée';
COMMENT ON COLUMN bugs_table.execution_action_id IS 'Identifiant unique de l''exécution de l''action de test associée';

COMMENT ON COLUMN bugs_table.creation_date IS 'Date de création de l''anomalie';
COMMENT ON COLUMN bugs_table.short_name IS 'Résumé de l''anomalie';
COMMENT ON COLUMN bugs_table.priority IS 'Priorité de l''anomalie';
COMMENT ON COLUMN bugs_table.severity IS 'Gravité de l''anomalie';
COMMENT ON COLUMN bugs_table.reproducibility IS 'Reproductibilité de l''anomalie';
COMMENT ON COLUMN bugs_table.platform IS 'Plateforme';
COMMENT ON COLUMN bugs_table.system IS 'Système d''exploitation';
COMMENT ON COLUMN bugs_table.description IS 'Description de l''anomalie';
COMMENT ON COLUMN bugs_table.status IS 'Status de l''anomalie';

COMMENT ON COLUMN bugs_table.bugtracker_bug_id IS 'Identifiant de l''anomalie dans le bugtracker';

CREATE VIEW bugs AS select * from bugs_table;

GRANT SELECT, UPDATE ON bugs_bug_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE bugs_table TO admin_role, writer_role;
GRANT SELECT ON TABLE bugs_table TO reader_role;
GRANT SELECT ON bugs TO admin_role, writer_role, reader_role;

-----------------------------------------
--	EXECUTIONS D'EXIGENCES DE CAMPAGNES
-----------------------------------------
CREATE SEQUENCE executions_requirements_execution_requirement_seq;

CREATE TABLE executions_requirements_table (
	execution_requirement_id bigint PRIMARY KEY DEFAULT nextval('executions_requirements_execution_requirement_seq'),
	execution_campaign_id bigint NOT NULL,
	test_content_id bigint NOT NULL,
	requirement_content_id bigint NOT NULL,
	result_id CHAR(1) NOT NULL,
	comments VARCHAR(16384),
	FOREIGN KEY (execution_campaign_id) REFERENCES executions_campaigns_table(execution_campaign_id) ON DELETE CASCADE,
	FOREIGN KEY(test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE,
	FOREIGN KEY(requirement_content_id) REFERENCES requirements_contents_table(requirement_content_id) ON DELETE CASCADE
);

COMMENT ON TABLE executions_requirements_table IS 'Exécutions d''exigences';
COMMENT ON COLUMN executions_requirements_table.execution_requirement_id IS 'Identifiant unique de l''exécution d''exigence';
COMMENT ON COLUMN executions_requirements_table.execution_campaign_id IS 'Identifiant de l''exécution de campagne';
COMMENT ON COLUMN executions_requirements_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN executions_requirements_table.requirement_content_id IS 'Identifiant du contenu d''exigence';
COMMENT ON COLUMN executions_requirements_table.result_id IS 'Identifiant du résultat';
COMMENT ON COLUMN executions_requirements_table.comments IS 'Commentaires';

CREATE VIEW executions_requirements AS select * from executions_requirements_table;

GRANT SELECT, UPDATE ON executions_requirements_execution_requirement_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_requirements_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_requirements_table TO reader_role;
GRANT SELECT ON executions_requirements TO admin_role, writer_role, reader_role;


-----------------------------------------
--	PIECES JOINTES DE TEST
-----------------------------------------
CREATE SEQUENCE tests_contents_files_test_content_file_id_seq;
CREATE TABLE tests_contents_files_table (
	test_content_file_id bigint PRIMARY KEY DEFAULT nextval('tests_contents_files_test_content_file_id_seq'),
	test_content_id bigint NOT NULL,
	test_content_filename VARCHAR(256) NOT NULL,
	test_content_lo_oid oid NOT NULL,
	FOREIGN KEY(test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE
);

COMMENT ON TABLE tests_contents_files_table IS 'Pièces jointes de tests';
COMMENT ON COLUMN tests_contents_files_table.test_content_file_id IS 'Identifiant unique de la pièce jointe';
COMMENT ON COLUMN tests_contents_files_table.test_content_id IS 'Identifiant du contenu de test';
COMMENT ON COLUMN tests_contents_files_table.test_content_filename IS 'Nom de la pièce jointe';
COMMENT ON COLUMN tests_contents_files_table.test_content_lo_oid IS 'Identifiant unique Large Object';

CREATE VIEW tests_contents_files AS select * from tests_contents_files_table;

GRANT SELECT, UPDATE ON tests_contents_files_test_content_file_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_contents_files_table TO admin_role, writer_role;
GRANT SELECT ON TABLE tests_contents_files_table TO reader_role;
GRANT SELECT ON tests_contents_files TO admin_role, writer_role, reader_role;

-----------------------------------------
--	DROITS SUR LES PROJETS
-----------------------------------------
CREATE TABLE projects_grants_table (
	project_id bigint NOT NULL,
  username varchar(64) NOT NULL,
  manage_tests_indic CHAR(1) NOT NULL DEFAULT 'N',
  manage_requirements_indic CHAR(1) NOT NULL DEFAULT 'N',
  manage_campaigns_indic CHAR(1) NOT NULL DEFAULT 'N',
  manage_executions_indic CHAR(1) NOT NULL DEFAULT 'N',
  PRIMARY KEY (project_id, username),
	FOREIGN KEY (project_id) REFERENCES projects_table (project_id) ON DELETE CASCADE
);

CREATE VIEW projects_grants AS select * from projects_grants_table;

COMMENT ON TABLE projects_grants_table IS 'Droits d''accès aux projets';
COMMENT ON COLUMN projects_grants_table.project_id IS 'Identifiant du projet';
COMMENT ON COLUMN projects_grants_table.username IS 'Identifiant utilisateur';
COMMENT ON COLUMN projects_grants_table.manage_tests_indic IS 'Droit de gestion des scenarios';
COMMENT ON COLUMN projects_grants_table.manage_requirements_indic IS 'Droit de gestion des exigences';
COMMENT ON COLUMN projects_grants_table.manage_campaigns_indic IS 'Droit de gestion des campagnes';
COMMENT ON COLUMN projects_grants_table.manage_executions_indic IS 'Droit de gestion des executions';

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE projects_grants_table TO admin_role;
GRANT SELECT ON TABLE projects_grants_table TO writer_role, reader_role;
GRANT SELECT ON projects_grants TO admin_role, writer_role, reader_role;

-- Donner les droits d'ecriture sur tous les projets pour l'administrateur dba
INSERT INTO projects_grants_table (project_id, username, manage_tests_indic, manage_requirements_indic, manage_campaigns_indic, manage_executions_indic)
	SELECT project_id, 'dba', 'W', 'W', 'W', 'W' FROM projects_table;

-----------------------------------------
-- STATUS DES CONTENUS D'EXIGENCES ET DES CONTENUS DE TESTS
-----------------------------------------
CREATE TABLE status_table (
	status_id CHAR(1) PRIMARY KEY,
	status_label VARCHAR(64) NOT NULL
);


COMMENT ON TABLE status_table IS 'Status des contenus d''exigences et des contenus de tests';
COMMENT ON COLUMN status_table.status_id IS 'Identifiant du status';
COMMENT ON COLUMN status_table.status_label IS 'Libelle du status';

CREATE OR REPLACE VIEW status AS SELECT * FROM status_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE status_table TO admin_role;
GRANT SELECT ON status_table TO writer_role, reader_role;
GRANT SELECT ON status TO admin_role, writer_role, reader_role;

INSERT INTO status_table (status_id, status_label)
VALUES ('V', 'Valide');

INSERT INTO status_table (status_id, status_label)
VALUES ('I', 'Invalide');

INSERT INTO status_table (status_id, status_label)
VALUES ('C', 'En cours de validation');


-- ********************************************
-- PARAMETRES D'EXECUTIONS DE TESTS
-- ********************************************
CREATE SEQUENCE executions_tests_parameters_parameter_id_seq;

CREATE TABLE executions_tests_parameters_table (
  execution_test_parameter_id bigint NOT NULL DEFAULT nextval('executions_tests_parameters_parameter_id_seq'),
  execution_test_id bigint NOT NULL,
  parameter_name VARCHAR(256) NOT NULL,
  parameter_value VARCHAR(256),
  PRIMARY KEY (execution_test_parameter_id),
  FOREIGN KEY (execution_test_id) REFERENCES executions_tests_table(execution_test_id) ON DELETE CASCADE
);

COMMENT ON TABLE executions_tests_parameters_table IS 'Paramètres d''exécutions de tests';
COMMENT ON COLUMN executions_tests_parameters_table.execution_test_parameter_id IS 'Identifiant unique du paramètre';
COMMENT ON COLUMN executions_tests_parameters_table.execution_test_id IS 'Identifiant de l''exécution de test associée';
COMMENT ON COLUMN executions_tests_parameters_table.parameter_name IS 'Nom du paramètre';
COMMENT ON COLUMN executions_tests_parameters_table.parameter_value IS 'Valeur du paramètre';

CREATE VIEW executions_tests_parameters AS select * from executions_tests_parameters_table;

GRANT SELECT, UPDATE ON executions_tests_parameters_parameter_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE executions_tests_parameters_table TO admin_role, writer_role;
GRANT SELECT ON TABLE executions_tests_parameters_table TO reader_role;
GRANT SELECT ON executions_tests_parameters TO admin_role, writer_role, reader_role;


-- Custom fields
-- entity types are : T=>test or R=>requirement
-- field types are : T=>text, I=>integer, F=>float, B=>boolean, L=>list, R=>Exclusive list
CREATE SEQUENCE custom_fields_desc_id_seq;

CREATE TABLE custom_fields_desc_table (
  custom_field_desc_id bigint NOT NULL DEFAULT nextval('custom_fields_desc_id_seq'),
  entity_type char(1) NOT NULL DEFAULT 'T',
  tab_name VARCHAR(128) NOT NULL,
  field_label VARCHAR(128) NOT NULL,
  field_type char(1) NOT NULL DEFAULT 'T',
  mandatory char(1) NOT NULL DEFAULT 'N',
  default_value varchar(16384),
  comma_separated_values varchar(16384),
  PRIMARY KEY (custom_field_desc_id)
);

CREATE OR REPLACE VIEW custom_fields_desc AS select * from custom_fields_desc_table;

GRANT SELECT, UPDATE ON custom_fields_desc_id_seq TO admin_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE custom_fields_desc_table TO admin_role;
GRANT SELECT ON TABLE custom_fields_desc_table TO writer_role, reader_role;
GRANT SELECT ON custom_fields_desc TO admin_role, writer_role, reader_role;

-- Custom test fields
CREATE SEQUENCE custom_test_fields_id_seq;

CREATE TABLE custom_test_fields_table (
  custom_test_field_id bigint NOT NULL DEFAULT nextval('custom_test_fields_id_seq'),
  custom_field_desc_id bigint NOT NULL,
  test_content_id bigint NOT NULL,
  field_value varchar(16384),
  PRIMARY KEY (custom_test_field_id),
  FOREIGN KEY (custom_field_desc_id) REFERENCES custom_fields_desc_table(custom_field_desc_id) ON DELETE CASCADE,
  FOREIGN KEY (test_content_id) REFERENCES tests_contents_table(test_content_id) ON DELETE CASCADE
);
CREATE OR REPLACE VIEW custom_test_fields AS select * from custom_test_fields_table;

GRANT SELECT, UPDATE ON custom_test_fields_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE custom_test_fields_table TO admin_role, writer_role;
GRANT SELECT ON TABLE custom_test_fields_table TO reader_role;
GRANT SELECT ON custom_test_fields TO admin_role, writer_role, reader_role;

-- Custom requirement fields
CREATE SEQUENCE custom_requirement_fields_id_seq;

CREATE TABLE custom_requirement_fields_table (
  custom_requirement_field_id bigint NOT NULL DEFAULT nextval('custom_requirement_fields_id_seq'),
  custom_field_desc_id bigint NOT NULL,
  requirement_content_id bigint NOT NULL,
  field_value varchar(16384),
  PRIMARY KEY (custom_requirement_field_id),
  FOREIGN KEY (custom_field_desc_id) REFERENCES custom_fields_desc_table(custom_field_desc_id) ON DELETE CASCADE,
  FOREIGN KEY (requirement_content_id) REFERENCES requirements_contents_table(requirement_content_id) ON DELETE CASCADE
);
CREATE OR REPLACE VIEW custom_requirement_fields AS select * from custom_requirement_fields_table;

GRANT SELECT, UPDATE ON custom_requirement_fields_id_seq TO admin_role, writer_role;
GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE custom_requirement_fields_table TO admin_role, writer_role;
GRANT SELECT ON TABLE custom_requirement_fields_table TO reader_role;
GRANT SELECT ON custom_requirement_fields TO admin_role, writer_role, reader_role;

-----------------------------------------
-- Types de test
-----------------------------------------
CREATE TABLE tests_types_table (
	test_type_id CHAR(1) PRIMARY KEY,
	test_type_label VARCHAR(64) NOT NULL
);


COMMENT ON TABLE tests_types_table IS 'Types de tests';
COMMENT ON COLUMN tests_types_table.test_type_id IS 'Identifiant du type de test';
COMMENT ON COLUMN tests_types_table.test_type_label IS 'Libelle du type de test';

CREATE OR REPLACE VIEW tests_types AS SELECT * FROM tests_types_table;

GRANT SELECT, INSERT, UPDATE, DELETE ON TABLE tests_types_table TO admin_role;
GRANT SELECT ON tests_types_table TO writer_role, reader_role;
GRANT SELECT ON tests_types TO admin_role, writer_role, reader_role;

INSERT INTO tests_types_table (test_type_id, test_type_label)
VALUES ('N', 'Scénario nominal');

INSERT INTO tests_types_table (test_type_id, test_type_label)
VALUES ('A', 'Scénario alternatif');

INSERT INTO tests_types_table (test_type_id, test_type_label)
VALUES ('E', 'Scénario d''exception');
