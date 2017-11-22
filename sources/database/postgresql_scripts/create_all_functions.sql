CONNECT :var_dbname;
CREATE LANGUAGE plpgsql;

\i functions/copy_project_version.sql
\i functions/create_test_from_test.sql
\i functions/create_test_from_requirement.sql
\i functions/create_requirement_from_requirement.sql
\i functions/create_need_from_need.sql
\i functions/create_feature_from_feature.sql
\i functions/create_rule_from_rule.sql
\i functions/html_to_plain_text.sql
