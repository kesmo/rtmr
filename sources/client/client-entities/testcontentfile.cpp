/*****************************************************************************
Copyright (C) 2012 Emmanuel Jorge ejorge@free.fr

This file is part of R.T.M.R.

R.T.M.R is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

R.T.M.R is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with R.T.M.R.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "testcontentfile.h"

TestContentFile::TestContentFile() : Record(getEntityDef())
{
    _m_test_content = NULL;
    _m_content_buffer = NULL;
    _m_is_file_source_buffer = false;
}

TestContentFile::TestContentFile(TestContent *in_test_content) : Record(getEntityDef())
{
    _m_test_content = in_test_content;
    _m_content_buffer = NULL;
    _m_is_file_source_buffer = false;
}

/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* TestContentFile::getEntityDef() const
{
    return &tests_contents_files_table_def;
}


void TestContentFile::setContentBuffer(const char *in_buffer, long in_buffer_length, int in_buffer_forrmat)
{
    if (_m_content_buffer)
        free(_m_content_buffer);

    _m_content_buffer = NULL;

    if (in_buffer){
        _m_content_buffer = (char*)malloc(in_buffer_length);
        memcpy(_m_content_buffer, in_buffer, in_buffer_length);
    }
    _m_content_buffer_length = in_buffer_length;
    _m_content_buffer_format = in_buffer_forrmat;
}

void TestContentFile::setSourceFile(const char *in_filename)
{
    _m_is_file_source_buffer = true;
    setValueForKey(in_filename, TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);
}

int TestContentFile::saveRecord()
{
    int 		tmp_return = NOERR;
    char		tmp_file_id_str[16];
    char		tmp_basename[LMEDIUM_TEXT_SIZE];
    const char		*tmp_basename_ptr = NULL;
    const char	*tmp_filename_ptr = NULL;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_test_content != NULL)
            setValueForKey(_m_test_content->getIdentifier(), TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_ID);

        tmp_filename_ptr = getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);
        if (NULL == _m_content_buffer)
        {
            if (_m_is_file_source_buffer)
            {
                tmp_return = cl_put_blob(CLIENT_SESSION, tmp_filename_ptr, tmp_file_id_str);
                if (tmp_return == NOERR)
                {
                    tmp_basename_ptr = strrchr(tmp_filename_ptr, '/');
                    strcpy(tmp_basename, tmp_basename_ptr + 1);
                }
            }
        }
        else
        {
            tmp_return = cl_put_blob_from_buffer(CLIENT_SESSION, _m_content_buffer, _m_content_buffer_length, tmp_file_id_str);
            if (tmp_return == NOERR)
            {
                if (is_empty_string(tmp_filename_ptr))
                    sprintf(tmp_basename, "attachment_%i_%s", _m_content_buffer_format, tmp_file_id_str);
                else
                    strcpy(tmp_basename, tmp_filename_ptr);

            }
        }

        if (tmp_return == NOERR)
        {
            if (NULL != _m_content_buffer || _m_is_file_source_buffer)
            {
                setValueForKey(tmp_basename, TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);
                setValueForKey(tmp_file_id_str, TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID);
                _m_is_file_source_buffer = false;
            }
            tmp_return = Record::insertRecord();
        }
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}

int TestContentFile::deleteRecord()
{
    int 		tmp_result = NOERR;

    if (is_empty_string(getIdentifier()) == FALSE && is_empty_string(getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_ID)) == FALSE)
    {
        tmp_result = cl_delete_blob(CLIENT_SESSION, getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID));
        if (tmp_result == NOERR || tmp_result == FILE_NOT_EXISTS){
            tmp_result = Record::deleteRecord();
        }
    }

    if (_m_content_buffer){
        free(_m_content_buffer);
        _m_content_buffer = NULL;
    }

    return tmp_result;
}


TestContentFile* TestContentFile::copy(TestContent *in_test_content)
{
    TestContentFile  *tmp_new_file = new TestContentFile(in_test_content);

    tmp_new_file->setValueForKey(getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID),TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID);
    tmp_new_file->setValueForKey(getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME),TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);
    tmp_new_file->setContentBuffer(_m_content_buffer, _m_content_buffer_length, _m_content_buffer_format);
    tmp_new_file->_m_is_file_source_buffer = _m_is_file_source_buffer;

    return tmp_new_file;
}
