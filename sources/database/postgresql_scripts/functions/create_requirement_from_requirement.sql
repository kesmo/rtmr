-- DROP FUNCTION create_requirement_from_requirement(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_requirement_id BIGINT, in_previous_requirement_id BIGINT, in_src_requirement_id BIGINT);
CREATE OR REPLACE FUNCTION create_requirement_from_requirement(in_project_id BIGINT, in_dst_version VARCHAR(12), in_parent_requirement_id BIGINT, in_previous_requirement_id BIGINT, in_src_requirement_id BIGINT, in_new_requirement_id BIGINT)
RETURNS BIGINT AS $$
DECLARE

	tmp_src_requirement_record RECORD;
	tmp_src_requirement_content_record RECORD;
	
	tmp_child_requirement_record RECORD;
	
	tmp_next_requirement_id BIGINT;
	
	tmp_requirement_content_id BIGINT;
	
	tmp_parent_requirement_id BIGINT;
	tmp_previous_requirement_id BIGINT;
	tmp_child_requirement_id BIGINT;

	tmp_new_requirement_id BIGINT;
	tmp_return BIGINT default 0;
	
BEGIN

	SELECT * INTO tmp_src_requirement_record FROM requirements_table WHERE requirement_id=in_src_requirement_id;
	IF FOUND THEN
	
		SELECT * INTO tmp_src_requirement_content_record FROM requirements_contents_table WHERE requirement_content_id=tmp_src_requirement_record.requirement_content_id;
		IF FOUND THEN
		
			IF in_previous_requirement_id IS NULL THEN
				SELECT requirement_id INTO tmp_next_requirement_id FROM requirements_table WHERE parent_requirement_id=in_parent_requirement_id AND previous_requirement_id IS NULL;
			ELSE
				SELECT requirement_id INTO tmp_next_requirement_id FROM requirements_table WHERE parent_requirement_id=in_parent_requirement_id AND previous_requirement_id=in_previous_requirement_id;
			END IF;
			
			INSERT INTO requirements_contents_table (short_name, description, category_id, priority_level, project_id, version)
			VALUES (
				tmp_src_requirement_content_record.short_name,
				tmp_src_requirement_content_record.description,
				tmp_src_requirement_content_record.category_id,
				tmp_src_requirement_content_record.priority_level,
				in_project_id,
				in_dst_version);
			SELECT currval('requirements_contents_requirement_content_id_seq') INTO tmp_requirement_content_id;			
			
			INSERT INTO requirements_table (requirement_content_id, parent_requirement_id, previous_requirement_id, project_id, version)
			VALUES (tmp_requirement_content_id, in_parent_requirement_id, in_previous_requirement_id, in_project_id, in_dst_version);
			SELECT currval('requirements_requirement_id_seq') INTO tmp_return;
			
			IF tmp_next_requirement_id IS NOT NULL THEN
				UPDATE requirements_table SET previous_requirement_id=tmp_return WHERE requirement_id=tmp_next_requirement_id;
			END IF;
			
			tmp_parent_requirement_id := tmp_src_requirement_record.requirement_id;
			tmp_child_requirement_id := NULL;
			tmp_previous_requirement_id := NULL;
			IF in_new_requirement_id IS NULL THEN
				tmp_new_requirement_id:=tmp_return;
			ELSE
				tmp_new_requirement_id:=in_new_requirement_id;
			END IF;
			
			SELECT * INTO tmp_child_requirement_record FROM requirements_table WHERE parent_requirement_id=tmp_parent_requirement_id AND previous_requirement_id IS NULL;
			IF FOUND AND tmp_child_requirement_record.requirement_id=tmp_new_requirement_id THEN
				SELECT * INTO tmp_child_requirement_record FROM requirements_table WHERE parent_requirement_id=tmp_parent_requirement_id AND previous_requirement_id=tmp_new_requirement_id;
			END IF;

			WHILE FOUND LOOP
			
				tmp_child_requirement_id := create_requirement_from_requirement(in_project_id, in_dst_version, tmp_return, tmp_child_requirement_id, tmp_child_requirement_record.requirement_id, tmp_return);
				tmp_previous_requirement_id := tmp_child_requirement_record.requirement_id;
				SELECT * INTO tmp_child_requirement_record FROM requirements_table WHERE parent_requirement_id=tmp_parent_requirement_id AND previous_requirement_id=tmp_previous_requirement_id;
				IF FOUND AND tmp_child_requirement_record.requirement_id=tmp_new_requirement_id THEN
					SELECT * INTO tmp_child_requirement_record FROM requirements_table WHERE parent_requirement_id=tmp_parent_requirement_id AND previous_requirement_id=tmp_new_requirement_id;
				END IF;
			END LOOP;
				
		ELSE
		
			RAISE NOTICE 'Pas de contenu d''exigence %', tmp_src_requirement_record.requirement_content_id;
			
		END IF;
		
	ELSE
	
		RAISE NOTICE 'Pas d''exigence %', in_src_requirement_id;
		
	END IF;
	
	return tmp_return;

END;
$$ LANGUAGE plpgsql;
