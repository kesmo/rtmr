--DROP FUNCTION copy_project_version(in_project_id BIGINT, in_src_version  VARCHAR(12), in_dst_version VARCHAR(12));
CREATE OR REPLACE FUNCTION copy_project_version(in_project_id BIGINT, in_src_version VARCHAR(12), in_dst_version VARCHAR(12))
RETURNS INT AS $$
DECLARE

	tmp_test_record RECORD;
	tmp_requirement_record RECORD;
	tmp_campaign_record RECORD;
	tmp_test_campaign_record RECORD;
	tmp_id BIGINT;

	tmp_original_id BIGINT;
	tmp_parent_id BIGINT;
	tmp_previous_id BIGINT;

	tmp_return INT default 0;
BEGIN


	CREATE TEMPORARY TABLE TEST_ID_MATRIX
	(
		src_test_id BIGINT PRIMARY KEY,
		dst_test_id BIGINT
	)
	ON COMMIT DROP;

	CREATE TEMPORARY TABLE REQUIREMENT_ID_MATRIX
	(
		src_requirement_id BIGINT PRIMARY KEY,
		dst_requirement_id BIGINT
	)
	ON COMMIT DROP;

	-- Creer la version
	INSERT INTO projects_versions_table (project_id, version) VALUES (in_project_id, in_dst_version);

	-- Copier tous les tests
	FOR tmp_test_record IN SELECT * FROM tests_table WHERE project_id=in_project_id AND version=in_src_version LOOP
		INSERT INTO tests_table (original_test_id, parent_test_id, previous_test_id, test_content_id, project_id, version)
			VALUES (tmp_test_record.original_test_id, tmp_test_record.parent_test_id, tmp_test_record.previous_test_id, tmp_test_record.test_content_id, in_project_id, in_dst_version);
		SELECT currval('tests_test_id_seq') INTO tmp_id;
		INSERT INTO TEST_ID_MATRIX (src_test_id, dst_test_id) VALUES (tmp_test_record.test_id, tmp_id);
	END LOOP;

	FOR tmp_test_record IN SELECT * FROM tests_table WHERE project_id=in_project_id AND version=in_dst_version LOOP

		tmp_original_id := NULL;
		tmp_parent_id := NULL;
		tmp_previous_id := NULL;

		-- Identifiant du test original
		IF tmp_test_record.original_test_id IS NOT NULL THEN
			SELECT dst_test_id INTO tmp_original_id FROM TEST_ID_MATRIX WHERE src_test_id=tmp_test_record.original_test_id;
		END IF;

		-- Identifiant du test parent
		IF tmp_test_record.parent_test_id IS NOT NULL THEN
			SELECT dst_test_id INTO tmp_parent_id FROM TEST_ID_MATRIX WHERE src_test_id=tmp_test_record.parent_test_id;
		END IF;

		-- Identifiant du test précédent
		IF tmp_test_record.previous_test_id IS NOT NULL THEN
			SELECT dst_test_id INTO tmp_previous_id FROM TEST_ID_MATRIX WHERE src_test_id=tmp_test_record.previous_test_id;
		END IF;

		UPDATE tests_table SET original_test_id=tmp_original_id, parent_test_id=tmp_parent_id, previous_test_id=tmp_previous_id WHERE test_id=tmp_test_record.test_id;

		SELECT dst_test_id INTO tmp_original_id FROM TEST_ID_MATRIX WHERE src_test_id=tmp_test_record.original_test_id;
		
	END LOOP;

	-- Copier toutes les exigences
	FOR tmp_requirement_record IN SELECT * FROM requirements_table WHERE project_id=in_project_id AND version=in_src_version LOOP
		INSERT INTO requirements_table (parent_requirement_id, previous_requirement_id, requirement_content_id, project_id, version)
			VALUES (tmp_requirement_record.parent_requirement_id, tmp_requirement_record.previous_requirement_id, tmp_requirement_record.requirement_content_id, in_project_id, in_dst_version);
		SELECT currval('requirements_requirement_id_seq') INTO tmp_id;
		INSERT INTO REQUIREMENT_ID_MATRIX (src_requirement_id, dst_requirement_id) VALUES (tmp_requirement_record.requirement_id, tmp_id);
	END LOOP;

	FOR tmp_requirement_record IN SELECT * FROM requirements_table WHERE project_id=in_project_id AND version=in_dst_version LOOP

		tmp_parent_id := NULL;
		tmp_previous_id := NULL;

		-- Identifiant de l'exigence parente
		IF tmp_requirement_record.parent_requirement_id IS NOT NULL THEN
			SELECT dst_requirement_id INTO tmp_parent_id FROM REQUIREMENT_ID_MATRIX WHERE src_requirement_id=tmp_requirement_record.parent_requirement_id;
		END IF;

		-- Identifiant de l'exigence précédente
		IF tmp_requirement_record.previous_requirement_id IS NOT NULL THEN
			SELECT dst_requirement_id INTO tmp_previous_id FROM REQUIREMENT_ID_MATRIX WHERE src_requirement_id=tmp_requirement_record.previous_requirement_id;
		END IF;

		UPDATE requirements_table SET parent_requirement_id=tmp_parent_id, previous_requirement_id=tmp_previous_id WHERE requirement_id=tmp_requirement_record.requirement_id;

	END LOOP;

	-- Copier toutes les campagnes
	FOR tmp_campaign_record IN SELECT * FROM campaigns_table WHERE project_id=in_project_id AND version=in_src_version LOOP

		INSERT INTO campaigns_table (project_id, version, short_name, description)
			VALUES (in_project_id, in_dst_version, tmp_campaign_record.short_name , tmp_campaign_record.description);
		SELECT currval('campaigns_campaign_id_seq') INTO tmp_id;

		-- Creer les tests de campagnes
		tmp_previous_id := NULL;
		FOR tmp_test_campaign_record IN SELECT * FROM tests_campaigns_table WHERE campaign_id=tmp_campaign_record.campaign_id LOOP
			SELECT dst_test_id INTO tmp_original_id FROM TEST_ID_MATRIX WHERE src_test_id=tmp_test_campaign_record.test_id;
			IF tmp_original_id IS NOT NULL THEN
				INSERT INTO tests_campaigns_table (campaign_id, test_id, previous_test_campaign_id)
					VALUES (tmp_id, tmp_original_id, tmp_previous_id);
				SELECT currval('tests_campaigns_test_campaign_id_seq') INTO tmp_previous_id;
			END IF;
		END LOOP;

	END LOOP;


	return tmp_return;

END;
$$ LANGUAGE plpgsql;
