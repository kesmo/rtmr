-- DROP FUNCTION create_test_from_test(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_test_id BIGINT, in_previous_test_id BIGINT, in_src_test_id BIGINT);
CREATE OR REPLACE FUNCTION create_test_from_test(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_test_id BIGINT, in_previous_test_id BIGINT, in_src_test_id BIGINT, in_new_test_id BIGINT)
RETURNS BIGINT AS $$
DECLARE

	tmp_src_test_record RECORD;
	tmp_src_test_content_record RECORD;
	
	tmp_child_test_record RECORD;
	tmp_action_record RECORD;
	tmp_requirement_content_record RECORD;
	
	tmp_next_test_id BIGINT;
	
	tmp_test_content_id BIGINT;
	tmp_previous_action_id BIGINT;
	tmp_action_id BIGINT;
	
	tmp_parent_test_id BIGINT;
	tmp_previous_test_id BIGINT;
	tmp_original_test_id BIGINT;
	tmp_child_test_id BIGINT;

	tmp_new_test_id BIGINT;
	tmp_return BIGINT default 0;
	
BEGIN

	SELECT * INTO tmp_src_test_record FROM tests_table WHERE test_id=in_src_test_id;
	IF FOUND THEN
	
		SELECT * INTO tmp_src_test_content_record FROM tests_contents_table WHERE test_content_id=tmp_src_test_record.test_content_id;
		IF FOUND THEN
		
			IF in_previous_test_id IS NULL THEN
				SELECT test_id INTO tmp_next_test_id FROM tests_table WHERE parent_test_id=in_parent_test_id AND previous_test_id IS NULL;
			ELSE
				SELECT test_id INTO tmp_next_test_id FROM tests_table WHERE parent_test_id=in_parent_test_id AND previous_test_id=in_previous_test_id;
			END IF;
			
			IF tmp_src_test_record.project_id=in_project_id AND tmp_src_test_record.version=in_dst_version AND tmp_src_test_record.original_test_id IS NOT NULL THEN

				tmp_test_content_id := tmp_src_test_record.test_content_id;
				tmp_original_test_id := tmp_src_test_record.original_test_id;
				
			ELSE
			
				INSERT INTO tests_contents_table (short_name, description, category_id, priority_level, project_id, version)
				VALUES (
					tmp_src_test_content_record.short_name,
					tmp_src_test_content_record.description,
					tmp_src_test_content_record.category_id,
					tmp_src_test_content_record.priority_level,
					in_project_id,
					in_dst_version);
				SELECT currval('tests_contents_test_content_id_seq') INTO tmp_test_content_id;

				tmp_previous_action_id := NULL;
				
				SELECT * INTO tmp_action_record FROM actions_table
				WHERE test_content_id=tmp_src_test_record.test_content_id AND previous_action_id IS NULL;

				WHILE FOUND LOOP
					tmp_action_id := tmp_action_record.action_id;

					INSERT INTO actions_table (test_content_id, previous_action_id, short_name, description, wait_result, link_original_test_content_id)
					VALUES (tmp_test_content_id, tmp_previous_action_id, tmp_action_record.short_name, tmp_action_record.description, tmp_action_record.wait_result, tmp_action_record.link_original_test_content_id);
					IF FOUND THEN
						SELECT currval('actions_action_id_seq') INTO tmp_previous_action_id;
						SELECT * INTO tmp_action_record FROM actions_table WHERE test_content_id=tmp_src_test_record.test_content_id AND previous_action_id=tmp_action_id;
					END IF;
					
				END LOOP;

				IF tmp_src_test_record.project_id=in_project_id THEN
				
					FOR tmp_requirement_content_record IN SELECT * FROM tests_requirements_table WHERE test_content_id=tmp_src_test_record.test_content_id LOOP
						INSERT INTO tests_requirements_table (test_content_id, original_requirement_content_id)
						VALUES (tmp_test_content_id, tmp_requirement_content_record.original_requirement_content_id);
					END LOOP;
					
				END IF;

			END IF;
			
			INSERT INTO tests_table (test_content_id, original_test_id, parent_test_id, previous_test_id, project_id, version) VALUES (tmp_test_content_id, tmp_original_test_id, in_parent_test_id, in_previous_test_id, in_project_id, in_dst_version);
			SELECT currval('tests_test_id_seq') INTO tmp_return;
			
			IF tmp_next_test_id IS NOT NULL THEN
				UPDATE tests_table SET previous_test_id=tmp_return WHERE test_id=tmp_next_test_id;
			END IF;
			
			IF tmp_original_test_id IS NULL THEN
			
				tmp_parent_test_id := tmp_src_test_record.test_id;
				tmp_child_test_id := NULL;
				tmp_previous_test_id := NULL;
				IF in_new_test_id IS NULL THEN
					tmp_new_test_id:=tmp_return;
				ELSE
					tmp_new_test_id:=in_new_test_id;
				END IF;
				
				SELECT * INTO tmp_child_test_record FROM tests_table WHERE parent_test_id=tmp_parent_test_id AND previous_test_id IS NULL;
				IF FOUND AND tmp_child_test_record.test_id=tmp_new_test_id THEN
					SELECT * INTO tmp_child_test_record FROM tests_table WHERE parent_test_id=tmp_parent_test_id AND previous_test_id=tmp_new_test_id;
				END IF;
	
				WHILE FOUND LOOP
				
					tmp_child_test_id := create_test_from_test(in_project_id, in_dst_version, tmp_return, tmp_child_test_id, tmp_child_test_record.test_id, tmp_return);
					tmp_previous_test_id := tmp_child_test_record.test_id;
					SELECT * INTO tmp_child_test_record FROM tests_table WHERE parent_test_id=tmp_parent_test_id AND previous_test_id=tmp_previous_test_id;
					IF FOUND AND tmp_child_test_record.test_id=tmp_new_test_id THEN
						SELECT * INTO tmp_child_test_record FROM tests_table WHERE parent_test_id=tmp_parent_test_id AND previous_test_id=tmp_new_test_id;
					END IF;
					
				END LOOP;
				
			END IF;
			
		ELSE
		
			RAISE NOTICE 'Pas de contenu de test %', tmp_src_test_record.test_content_id;
			
		END IF;
		
	ELSE
	
		RAISE NOTICE 'Pas de test %', in_src_test_id;
		
	END IF;
	
	return tmp_return;

END;
$$ LANGUAGE plpgsql;
