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

#ifndef TESTCONTENTFILE_H_
#define TESTCONTENTFILE_H_

#include "record.h"
#include "testcontent.h"

class TestContentFile: public Record
{
private:
    TestContent  *_m_test_content;
    char	    *_m_content_buffer;
    long	    _m_content_buffer_length;
    int		    _m_content_buffer_format;
    bool        _m_is_file_source_buffer;

public:
	TestContentFile();
	TestContentFile(TestContent *in_test_content);
	~TestContentFile(){destroy();}

    const entity_def* getEntityDef() const;

    void setTestContent(TestContent *in_test_content){_m_test_content = in_test_content;}
    TestContent* testContent(){return _m_test_content;}

    void setContentBuffer(const char *in_buffer, long in_buffer_length, int in_buffer_forrmat);
    void setSourceFile(const char *in_filename);
    const char* getContentBuffer() const {return _m_content_buffer;}
    long getContentBufferLength() const {return _m_content_buffer_length;}

    int saveRecord();
    int deleteRecord();

    TestContentFile* copy(TestContent *in_test_content);
};

#endif /* TESTCONTENTFILE_H_ */
