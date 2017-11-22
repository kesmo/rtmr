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

#ifndef TESTACTIONATTACHMENTSMANAGER_H
#define TESTACTIONATTACHMENTSMANAGER_H

#include "testcontent.h"
#include "testcontentfile.h"
#include "gui/components/record_text_edit.h"

class TestActionAttachmentsManager
{
public:
    TestActionAttachmentsManager();
    ~TestActionAttachmentsManager();

    virtual void loadTextEditAttachments(RecordTextEdit *in_text_edit);
    virtual void loadTextEditAttachments(QString &in_html, QTextDocument* in_document, QString images_folder = QString());

    virtual int saveTextEditAttachments(TestContent *in_test_content, RecordTextEdit *in_text_edit);

protected:
    QList<TestContentFile*>	_m_files;

    void destroyAttachments();

private:
    QMap<QString, char*>	_m_actions_buffers_attachments;

};

#endif // TESTACTIONATTACHMENTSMANAGER_H
