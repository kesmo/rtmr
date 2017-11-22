\set ON_ERROR_STOP on

CREATE DATABASE :var_dbname;

\connect :var_dbname

\i create_database.sql
\i functions/copy_project_version.sql
\i functions/create_test_from_test.sql
\i functions/create_test_from_requirement.sql
\i functions/create_requirement_from_requirement.sql
\i functions/html_to_plain_text.sql
