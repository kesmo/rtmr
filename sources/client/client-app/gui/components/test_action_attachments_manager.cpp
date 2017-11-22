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

#include "test_action_attachments_manager.h"
#include "session.h"

#include <QDomDocument>

TestActionAttachmentsManager::TestActionAttachmentsManager()
{
}


TestActionAttachmentsManager::~TestActionAttachmentsManager()
{
    destroyAttachments();
}

void TestActionAttachmentsManager::destroyAttachments()
{
    foreach(char* tmp_buffer, _m_actions_buffers_attachments.values())
    {
        cl_free_str(&tmp_buffer);
    }
    _m_actions_buffers_attachments.clear();

    qDeleteAll(_m_files);
    _m_files.clear();
}



void TestActionAttachmentsManager::loadTextEditAttachments(RecordTextEdit *in_text_edit)
{
    QString tmp_html = in_text_edit->toHtml();
    loadTextEditAttachments(tmp_html, in_text_edit->document());
}

void TestActionAttachmentsManager::loadTextEditAttachments(QString &in_html, QTextDocument* in_document, QString images_folder)
{
    char                *tmp_resource_buffer = NULL;
    QDomDocument        tmp_xml_doc;
    tmp_xml_doc.setContent(in_html);
    QDomNodeList        tmp_img_list = tmp_xml_doc.elementsByTagName("img");
    const char          *tmp_bytes_array = NULL;

    if (!tmp_img_list.isEmpty())
    {
        for(int tmp_node_index = 0; tmp_node_index < tmp_img_list.count(); tmp_node_index++)
        {
            QDomElement tmp_img = tmp_img_list.item(tmp_node_index).toElement();
            if (!tmp_img.isNull())
            {
                QDomAttr tmp_img_url = tmp_img.attributeNode("src");
                if (!tmp_img_url.isNull())
                {
                    for(int tmp_index = 0; tmp_index < _m_files.count(); tmp_index++)
                    {
                        if (strcmp(_m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME), tmp_img_url.value().toStdString().c_str()) == 0)
                        {
                            tmp_resource_buffer = _m_actions_buffers_attachments.value(tmp_img_url.value(), NULL);
                            if (cl_get_blob_to_buffer(Session::instance()->getClientSession(), _m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID), &tmp_resource_buffer, 0) == NOERR)
                            {
                                QDomAttr tmp_img_width = tmp_img.attributeNode("width");
                                QDomAttr tmp_img_height = tmp_img.attributeNode("height");

                                QStringList tmp_file_strings = QString(_m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME)).split('_');
                                if (tmp_file_strings.count() == 3)
                                {
                                    if (!tmp_img_width.isNull() && !tmp_img_height.isNull())
                                    {
                                        QImage tmp_image_buffer((uchar*)tmp_resource_buffer, tmp_img_width.value().toInt(), tmp_img_height.value().toInt(), (QImage::Format)tmp_file_strings[1].toInt());
                                        if (images_folder.isEmpty())
                                        {
                                            in_document->addResource(QTextDocument::ImageResource, QUrl(_m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME)), tmp_image_buffer);
                                            tmp_bytes_array = (const char*)tmp_image_buffer.bits();

                                            _m_files[tmp_index]->setContentBuffer(tmp_bytes_array, tmp_image_buffer.byteCount(), tmp_image_buffer.format());

                                            tmp_img.setAttribute("width", 48);
                                            tmp_img.setAttribute("height", tmp_image_buffer.height() * 48 / tmp_image_buffer.width());
                                        }
                                        else
                                        {
                                            tmp_img_url.setValue(QDir(images_folder).dirName()+"/"+_m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME)+".png");
                                            tmp_image_buffer.save(images_folder+"/"+_m_files[tmp_index]->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME)+".png");
                                        }
                                    }
                                }

                                _m_actions_buffers_attachments.insert(tmp_img_url.value(), tmp_resource_buffer);
                            }
                        }
                    }
                }
            }
        }

        if (!images_folder.isEmpty())
        {
            in_html = tmp_xml_doc.toString(0).remove('\n');
        }

    }
}


int TestActionAttachmentsManager::saveTextEditAttachments(TestContent *in_test_content, RecordTextEdit *in_text_edit)
{
    QMap<QString, QImage> tmp_action_attachments;
    const char          *tmp_bytes_array = NULL;
    TestContentFile	*tmp_file = NULL;
    int			tmp_save_result = NOERR;

    if (NULL != in_text_edit)
    {
        QDomDocument tmp_doc;
        QString tmp_html = in_text_edit->toHtml();
        tmp_doc.setContent(tmp_html);
        QDomNodeList tmp_img_list = tmp_doc.elementsByTagName("img");
        if (!tmp_img_list.isEmpty())
        {
            tmp_action_attachments = in_text_edit->images();
            QMapIterator<QString, QImage> tmp_action_attachments_iterator(tmp_action_attachments);

            for(int tmp_node_index = 0; tmp_node_index < tmp_img_list.count(); tmp_node_index++)
            {
                QDomElement tmp_img = tmp_img_list.item(tmp_node_index).toElement();
                if (!tmp_img.isNull())
                {
                    QDomAttr tmp_img_url = tmp_img.attributeNode("src");
                    if (!tmp_img_url.isNull())
                    {
                        tmp_action_attachments_iterator.toFront();
                        while(tmp_action_attachments_iterator.hasNext())
                        {
                            tmp_action_attachments_iterator.next();
                            if (tmp_img_url.value() == tmp_action_attachments_iterator.key())
                            {
                                QImage tmp_image = tmp_action_attachments_iterator.value();

                                tmp_file = new TestContentFile(in_test_content);
                                tmp_file->setValueForKey(tmp_action_attachments_iterator.key().toStdString().c_str(), TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);

                                tmp_bytes_array = (const char*)tmp_image.bits();
                                tmp_file->setContentBuffer(tmp_bytes_array, tmp_image.byteCount(), tmp_image.format());

                                tmp_save_result = tmp_file->saveRecord();
                                if (tmp_save_result == NOERR)
                                {
                                    tmp_img_url.setValue(tmp_file->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME));
                                    tmp_img.setAttribute("width", tmp_image.width());
                                    tmp_img.setAttribute("height", tmp_image.height());
                                }
                            }
                        }
                    }
                }
            }
        }

        // Contournement bug Qt qui rajoute un espace à la fin des paragraphes (balise <p>) s'ils sont suivis d'un saut de ligne
        tmp_html = tmp_doc.toString(0).remove('\n');
        in_text_edit->setHtml(tmp_html);
    }

    return tmp_save_result;
}
