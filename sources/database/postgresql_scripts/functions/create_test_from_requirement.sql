--DROP FUNCTION create_test_from_requirement(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_test_id BIGINT, in_previous_test_id BIGINT, in_src_requirement_id BIGINT);
CREATE OR REPLACE FUNCTION create_test_from_requirement(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_test_id BIGINT, in_previous_test_id BIGINT, in_src_requirement_id BIGINT)
RETURNS BIGINT AS $$
DECLARE

	tmp_requirement_record RECORD;
	tmp_requirement_content_record RECORD;
	
	tmp_test_id BIGINT;
	
	tmp_test_content_id BIGINT;
	
	tmp_parent_requirement_id BIGINT;
	tmp_previous_requirement_id BIGINT;
	tmp_previous_test_id BIGINT;

	tmp_return BIGINT default 0;
	
BEGIN

	SELECT * INTO tmp_requirement_record FROM requirements_table WHERE requirement_id=in_src_requirement_id;
	IF FOUND THEN
	
		SELECT * INTO tmp_requirement_content_record FROM requirements_contents_table WHERE requirement_content_id=tmp_requirement_record.requirement_content_id;
		IF FOUND THEN
		
			IF in_previous_test_id IS NULL THEN
				SELECT test_id INTO tmp_test_id FROM tests_table WHERE parent_test_id=in_parent_test_id AND previous_test_id IS NULL;
			ELSE
				SELECT test_id INTO tmp_test_id FROM tests_table WHERE parent_test_id=in_parent_test_id AND previous_test_id=in_previous_test_id;
			END IF;
			
			INSERT INTO tests_contents_table (short_name, description, category_id, priority_level, project_id, version)
			VALUES (
				tmp_requirement_content_record.short_name,
				tmp_requirement_content_record.description,
				tmp_requirement_content_record.category_id,
				tmp_requirement_content_record.priority_level,
				in_project_id,
				in_dst_version);
			SELECT currval('tests_contents_test_content_id_seq') INTO tmp_test_content_id;
			
			INSERT INTO tests_table (test_content_id, parent_test_id, previous_test_id, project_id, version)
			VALUES (tmp_test_content_id, in_parent_test_id, in_previous_test_id, in_project_id, in_dst_version);
			SELECT currval('tests_test_id_seq') INTO tmp_return;
			
			IF tmp_test_id IS NOT NULL THEN
				UPDATE tests_table SET previous_test_id=tmp_return WHERE test_id=tmp_test_id;
			END IF;
			
			IF tmp_requirement_record.project_id=in_project_id THEN
				INSERT INTO tests_requirements_table (test_content_id, original_requirement_content_id)
				VALUES (tmp_test_content_id, tmp_requirement_content_record.original_requirement_content_id);
			END IF;
			
			tmp_parent_requirement_id := tmp_requirement_record.requirement_id;
			tmp_previous_requirement_id := NULL;
			tmp_previous_test_id := NULL;
			
			SELECT * INTO tmp_requirement_record
				FROM requirements_table
				WHERE parent_requirement_id = tmp_parent_requirement_id AND previous_requirement_id IS NULL;

			WHILE FOUND LOOP
			
				tmp_previous_test_id := create_test_from_requirement(in_project_id, in_dst_version, tmp_return, tmp_previous_test_id, tmp_requirement_record.requirement_id);
				tmp_previous_requirement_id := tmp_requirement_record.requirement_id;
				SELECT * INTO tmp_requirement_record FROM requirements_table WHERE parent_requirement_id=tmp_parent_requirement_id AND previous_requirement_id=tmp_previous_requirement_id;
				
			END LOOP;
			
		ELSE
		
			RAISE NOTICE 'Pas de contenu d''exigence %', tmp_requirement_record.requirement_content_id;
			
		END IF;
		
	ELSE
	
		RAISE NOTICE 'Pas d''exigence %', in_src_requirement_id;
		
	END IF;
	
	return tmp_return;

END;
$$ LANGUAGE plpgsql;
