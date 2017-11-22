--DROP FUNCTION html_to_plain_text(in_html_string VARCHAR);

CREATE OR REPLACE FUNCTION html_to_plain_text(in_html_string VARCHAR)
RETURNS varchar AS $$
DECLARE

	tmp_return varchar default '';
	tmp_str varchar default '';
	tmp_start INTEGER DEFAULT 1;
	tmp_end INTEGER DEFAULT 1;

BEGIN

	tmp_start := strpos(in_html_string, '<body');
	IF tmp_start > 0 THEN
		tmp_str := substr(in_html_string, tmp_start);
		WHILE tmp_start > 0 AND tmp_end > 0 LOOP
			tmp_start := strpos(tmp_str, '<');
			IF tmp_start > 0 THEN
				tmp_return := tmp_return || substr(tmp_str, 1, tmp_start - 1);
				tmp_end := strpos(tmp_str, '>');
				IF tmp_end > 0 THEN
					tmp_str := substr(tmp_str, tmp_end + 1);
				END IF;
			END IF;
		END LOOP;
	ELSE
		tmp_return := in_html_string;
	END IF;	
	return tmp_return;

END;
$$ LANGUAGE plpgsql;
