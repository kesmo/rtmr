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

#include "requirementhierarchy.h"
#include "test.h"
#include "testhierarchy.h"
#include "requirementcontent.h"
#include "projectgrant.h"

#include "utilities.h"
#include "entities.h"
#include "session.h"

#include <QTextDocument>
#include <QIcon>
#include <QPixmap>

/**
  Constructeur
**/
RequirementHierarchy::RequirementHierarchy() : Hierarchy(getEntityDef())
{
    _m_project_version = NULL;
    _m_parent = NULL;
}


/**
  Constructeur
**/
RequirementHierarchy::RequirementHierarchy(ProjectVersion *in_project) : Hierarchy(getEntityDef())
{
    _m_parent = NULL;
    _m_project_version = NULL;
    setProjectVersion(in_project);
}


/**
  Constructeur
**/
RequirementHierarchy::RequirementHierarchy(RequirementHierarchy *in_parent) : Hierarchy(getEntityDef())
{
    _m_project_version = NULL;
    _m_parent = in_parent;
    if (_m_parent != NULL)
    {
        setProjectVersion(_m_parent->projectVersion());
    }
}


/**
  Destructeur
**/
RequirementHierarchy::~RequirementHierarchy()
{
    qDeleteAll(_m_childs);
    destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* RequirementHierarchy::getEntityDef() const
{
    return &requirements_hierarchy_def;
}

void RequirementHierarchy::setProjectVersion(ProjectVersion* in_project_version)
{
    if (in_project_version != NULL)
    {
        _m_project_version = in_project_version;
        if (is_empty_string(getIdentifier()))
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), REQUIREMENTS_HIERARCHY_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), REQUIREMENTS_HIERARCHY_VERSION);
        }
    }
}



/**
  Renvoie le requirement enfant
**/
Hierarchy* RequirementHierarchy::child(int number, int /*in_child_type*/)
{
    return _m_childs.value(number);
}



/**
  Renvoie le nombre de exigences filles
**/
int RequirementHierarchy::childCount(int /*in_child_type*/)
{
    return _m_childs.count();
}



/**
  Renvoie la valeur de la colonne <i>column</i>
**/
QVariant RequirementHierarchy::data(int /*column*/, int in_role)
{
    switch (in_role)
    {
    /* Icone */
    case Qt::DecorationRole:
        switch (_m_record_status)
        {
        case RECORD_STATUS_OUT_OF_SYNC:
            return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/clock.png")));
            break;

        case RECORD_STATUS_LOCKED:
            return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/lock.png")));
            break;

        case RECORD_STATUS_BROKEN:
            return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/notes_broken.png")));
            break;

        default:
            if(childCount() > 0)
                return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/folder_red.png")));
            else
                return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/notes.png")));
        }

        break;

        // Renvoie le titre du item courant
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
        return QVariant(QString(getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));
        break;

    default:
        return QVariant();

    }

    return QVariant();
}


bool RequirementHierarchy::insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type)
{
    RequirementHierarchy	*tmp_requirement = NULL;
    net_session			*tmp_session = CLIENT_SESSION;
    char			***tmp_results = NULL;

    unsigned long		tmp_rows_count, tmp_columns_count;

    bool                        tmp_return = false;

    if (in_item != NULL)
    {
        if (in_child_type == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            sprintf(tmp_session->m_last_query, "select create_requirement_from_requirement(%s, '%s', %s, %s, %s, NULL);",
                    _m_project_version->project()->getIdentifier(),
                    _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
                    getIdentifier(),
                    (position > 0 ? _m_childs[position -1]->getIdentifier() : "NULL"),
                    in_item->getIdentifier());
            tmp_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
            if (tmp_results != NULL)
            {
                if (tmp_rows_count == 1 && tmp_columns_count == 1)
                {
                    if (is_empty_string(tmp_results[0][0]) == FALSE)
                    {
                        tmp_requirement = new RequirementHierarchy(this);
                        if (tmp_requirement->loadRecord(tmp_results[0][0]) == NOERR)
                        {
                            if (insertChildren(position, count, tmp_requirement, in_child_type))
                            {
                                tmp_return = (tmp_requirement->loadChilds() == NOERR);
                            }
                        }
                    }
                }

                cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
            }
        }
    }

    return tmp_return;
}


/**
  Insertion d'un nouveau requirement enfant
**/
bool RequirementHierarchy::insertChildren(int in_index, int /* count */, Hierarchy *in_requirement, int /*in_child_type*/)
{
    RequirementHierarchy		*tmp_requirement = NULL;
    RequirementHierarchy		*tmp_next_requirement = NULL;
    RequirementContent*	tmp_requirement_content = NULL;

    int				tmp_save_result = NOERR;

    if (in_requirement == NULL)
    {
        tmp_requirement_content = new RequirementContent(_m_project_version);
        tmp_requirement_content->setValueForKey("", REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
        tmp_save_result = tmp_requirement_content->saveRecord();
        if (tmp_save_result == NOERR)
        {
            tmp_requirement = new RequirementHierarchy(_m_project_version);
            tmp_requirement->setDataFromRequirementContent(tmp_requirement_content);
        }
        delete tmp_requirement_content;
    }
    else
        tmp_requirement = (RequirementHierarchy*)in_requirement;

    if (tmp_save_result == NOERR)
    {
        tmp_requirement->setProjectVersion(_m_project_version);
        tmp_requirement->_m_parent = this;
        if (in_index > 0)
            tmp_requirement->setValueForKey(_m_childs[in_index - 1]->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
        else
            tmp_requirement->setValueForKey(NULL, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

        tmp_save_result = tmp_requirement->saveRecord();
        if (tmp_save_result == NOERR)
        {
            _m_childs.insert(in_index, tmp_requirement);

            // Traitement du suivant
            if (_m_childs.isEmpty() == false && in_index + 1 < _m_childs.size())
            {
                tmp_next_requirement = _m_childs[in_index + 1];
                if (tmp_next_requirement != NULL)
                {
                    tmp_next_requirement->setValueForKey(tmp_requirement->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
                    tmp_save_result = tmp_next_requirement->saveRecord();
                }
            }

            return (tmp_save_result == NOERR);
        }
    }

    return false;
}

bool RequirementHierarchy::setChilds(QList < RequirementHierarchy * > in_requirements_list)
{
    _m_childs = in_requirements_list;

    foreach(RequirementHierarchy *tmp_requirement, _m_childs)
    {
        tmp_requirement->_m_parent = this;
    }

    return true;
}


/**
  Renvoie une copie du requirement
**/
Hierarchy* RequirementHierarchy::copy()
{
    RequirementHierarchy*		tmp_copy = NULL;
    RequirementContent*	tmp_requirement_content = NULL;
    int				tmp_save_result = NOERR;

    tmp_requirement_content = new RequirementContent(_m_project_version);
    tmp_requirement_content->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_requirement_content->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PRIORITY_LEVEL), REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);
    tmp_requirement_content->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_CATEGORY_ID), REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);
    tmp_save_result = tmp_requirement_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_copy = new RequirementHierarchy(_m_project_version);
        tmp_copy->setDataFromRequirementContent(tmp_requirement_content);
    }
    delete tmp_requirement_content;

    return tmp_copy;
}


Hierarchy* RequirementHierarchy::cloneForMove(bool in_recursivly)
{
    RequirementHierarchy*    tmp_clone = clone<RequirementHierarchy>();
    RequirementHierarchy*    tmp_child_requirement = NULL;

    tmp_clone->_m_record_status = _m_record_status;
    tmp_clone->_m_project_version = _m_project_version;
    if (in_recursivly)
    {
        foreach(RequirementHierarchy *tmp_requirement, _m_childs)
        {
            tmp_child_requirement = (RequirementHierarchy*)tmp_requirement->cloneForMove(in_recursivly);
            tmp_child_requirement->_m_parent = tmp_clone;
            tmp_clone->_m_childs.append(tmp_child_requirement);
        }
    }

    return tmp_clone;

}


/**
  Renvoie le requirement parent
**/
Hierarchy *RequirementHierarchy::parent()
{
    if (_m_parent != NULL)
        return _m_parent;
    else
        return _m_project_version;
}

int RequirementHierarchy::row() const
{
    if (_m_parent)
        return _m_parent->_m_childs.indexOf(const_cast<RequirementHierarchy*>(this));
    else
        return _m_project_version->requirementsHierarchy().indexOf(const_cast<RequirementHierarchy*>(this));

    return 0;
}

/**
  Supprimer un requirement enfant
**/
bool RequirementHierarchy::removeChildren(int in_index, int count, bool in_move_indic, int /* in_child_type */)
{
    RequirementHierarchy *tmp_requirement = NULL;
    int		    tmp_save_result = NOERR;

    if (in_index + count < _m_childs.size())
    {
        if (in_index > 0)
            _m_childs[in_index + count]->setValueForKey(_m_childs[in_index - 1]->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
        else
            _m_childs[in_index + count]->setValueForKey(NULL, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

        tmp_save_result = _m_childs[in_index + count]->saveRecord();
    }

    if (tmp_save_result == NOERR)
    {
        for (int tmp_index = 0; tmp_index < count; tmp_index++)
        {
            tmp_requirement = _m_childs.takeAt(in_index);
            if (in_move_indic == false)
            {
                tmp_save_result = tmp_requirement->deleteRecord();
            }
        }
    }

    return (tmp_save_result == NOERR);
}


bool RequirementHierarchy::isWritable(int /* in_child_type */)
{
    if (_m_project_version == NULL || _m_project_version->project() == NULL || _m_project_version->project()->projectGrants() == NULL)
        return false;

    return compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_WRITE) == 0;
}


int RequirementHierarchy::saveRecord()
{
    int			tmp_result = NOERR;
    Requirement		*tmp_requirement_record = cloneRequirementRecord();

    if (_m_parent != NULL)
    {
        if (is_empty_string(_m_parent->getIdentifier()))
        {
            tmp_result = _m_parent->saveRecord();
            if (tmp_result == NOERR)
                setValueForKey(_m_parent->getIdentifier(), REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);
            else
                return tmp_result;
        }
        else
            setValueForKey(_m_parent->getIdentifier(), REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);

        setValueForKey(_m_parent->getValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID), REQUIREMENTS_HIERARCHY_PROJECT_ID);
        setValueForKey(_m_parent->getValueForKey(REQUIREMENTS_HIERARCHY_VERSION), REQUIREMENTS_HIERARCHY_VERSION);
    }
    else
        setValueForKey(NULL, REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);

    if (_m_project_version != NULL)
    {
        setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), REQUIREMENTS_HIERARCHY_PROJECT_ID);
        setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), REQUIREMENTS_HIERARCHY_VERSION);
    }

    tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID), REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID);
    tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID), REQUIREMENTS_TABLE_PREVIOUS_REQUIREMENT_ID);
    tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID), REQUIREMENTS_TABLE_PROJECT_ID);
    tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID), REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID);
    tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_VERSION), REQUIREMENTS_TABLE_VERSION);

    if (is_empty_string(getIdentifier()))
    {
        tmp_result = tmp_requirement_record->insertRecord();
        if (tmp_result == NOERR)
        {
            setValueForKey(tmp_requirement_record->getIdentifier(), REQUIREMENTS_HIERARCHY_REQUIREMENT_ID);
            cloneColumns();
        }
    }
    else
    {
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_ID), REQUIREMENTS_TABLE_REQUIREMENT_ID);
        tmp_result = tmp_requirement_record->saveRecord();
        if (tmp_result == NOERR)
        {
            cloneColumns();
        }
    }

    return tmp_result;
}


void RequirementHierarchy::searchFieldWithValue(QList<Record*> *in_found_list, const char* in_field_name, const char* in_field_value, bool in_recursive, int in_comparison_value)
{
    int		tmp_comparison = 0;

    tmp_comparison = compare_values(getValueForKey(in_field_name), in_field_value);
    if ((tmp_comparison == 0 && (in_comparison_value == EqualTo || in_comparison_value == LowerOrEqualTo || in_comparison_value == UpperOrEqualTo))
            || (tmp_comparison < 0 && (in_comparison_value == LowerThan || in_comparison_value == LowerOrEqualTo))
            || (tmp_comparison > 0 && (in_comparison_value == UpperThan || in_comparison_value == UpperOrEqualTo)))
    {
        in_found_list->append(this);
    }

    if (in_recursive)
    {
        foreach(RequirementHierarchy *tmp_child, _m_childs)
        {
            tmp_child->searchFieldWithValue(in_found_list, in_field_name, in_field_value, in_recursive, in_comparison_value);
        }
    }
}


int RequirementHierarchy::dbChildCount(int /*in_child_type*/)
{
    char			tmp_statement_str[256];
    char			***tmp_results = NULL;
    unsigned long	tmp_rows_count = 0;
    unsigned long	tmp_columns_count = 0;
    int				tmp_records_count = -1;

    if (is_empty_string(getIdentifier()) == false)
    {
        /* Verifier qu'il n'existe pas un nouvel element a la suite */
        sprintf(tmp_statement_str, "SELECT COUNT(%s) FROM %s WHERE %s=%s;"
                , REQUIREMENTS_TABLE_REQUIREMENT_ID
                , REQUIREMENTS_TABLE_SIG
                , REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID
                , getIdentifier());
        tmp_results = cl_run_sql(CLIENT_SESSION, tmp_statement_str, &tmp_rows_count, &tmp_columns_count);
        if (tmp_results != NULL)
        {
            tmp_records_count = atoi(tmp_results[0][0]);
            cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
        }
    }

    return tmp_records_count;
}



RequirementHierarchy* RequirementHierarchy::findRequirementWithId(const QList<RequirementHierarchy*>& in_requirements_list, const char* in_requirement_id, bool in_recursive)
{
    return findRequirementWithValueForKey(in_requirements_list, in_requirement_id, REQUIREMENTS_HIERARCHY_REQUIREMENT_ID,in_recursive);
}


int RequirementHierarchy::deleteRecord()
{
    Requirement		*tmp_requirement = NULL;
    int			tmp_result = NOERR;

    if (is_empty_string(getIdentifier()) == FALSE)
    {
        tmp_requirement = new Requirement();
        tmp_requirement->setValueForKey(getIdentifier(), REQUIREMENTS_TABLE_REQUIREMENT_ID);
        tmp_result = tmp_requirement->deleteRecord();
        delete tmp_requirement;
    }

    return tmp_result;
}

Requirement* RequirementHierarchy::cloneRequirementRecord()
{
    Requirement		*tmp_requirement_record = new Requirement();

    tmp_requirement_record->setValueForKey(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID), REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID);
    tmp_requirement_record->setValueForKey(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID), REQUIREMENTS_TABLE_PREVIOUS_REQUIREMENT_ID);
    tmp_requirement_record->setValueForKey(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID), REQUIREMENTS_TABLE_PROJECT_ID);
    tmp_requirement_record->setValueForKey(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID), REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID);
    tmp_requirement_record->setValueForKey(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_VERSION), REQUIREMENTS_TABLE_VERSION);
    tmp_requirement_record->cloneColumns();

    return tmp_requirement_record;
}


int RequirementHierarchy::loadChilds()
{
    RequirementHierarchy	**tmp_childs_requirements = NULL;
    unsigned long			tmp_requirements_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;
    unsigned long			tmp_index = 0;

    _m_childs.clear();

    sprintf(tmp_session->m_last_query, "%s=%s", REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID, getIdentifier());

    tmp_childs_requirements = Record::loadRecords<RequirementHierarchy>(tmp_session, &requirements_hierarchy_def, tmp_session->m_last_query, NULL, &tmp_requirements_count);
    if (tmp_childs_requirements != NULL)
    {
        if (tmp_requirements_count > 0)
        {
            for(tmp_index = 0; tmp_index < tmp_requirements_count; tmp_index++)
            {
                tmp_childs_requirements[tmp_index]->_m_parent = this;
                tmp_childs_requirements[tmp_index]->_m_project_version = _m_project_version;
                tmp_childs_requirements[tmp_index]->loadChilds();
            }
            _m_childs = ProjectVersion::orderedProjectRequirements(_m_project_version, tmp_childs_requirements, tmp_requirements_count, getIdentifier());
        }

        Record::freeRecords<RequirementHierarchy>(tmp_childs_requirements, 0);

    }

    return NOERR;
}


Hierarchy* RequirementHierarchy::findItemWithId(const char* in_item_id, int in_child_type, bool in_recursive)
{
    if (in_child_type == REQUIREMENTS_HIERARCHY_SIG_ID)
        return findRequirementWithId(_m_childs, in_item_id, in_recursive);

    return NULL;
}


RequirementHierarchy* RequirementHierarchy::findRequirementWithValueForKey(const QList<RequirementHierarchy*>& in_requirements_list, const char* in_value, const char* in_key, bool in_recursive)
{
    RequirementHierarchy *tmp_found_requirement = NULL;

    foreach(RequirementHierarchy *tmp_requirement, in_requirements_list)
    {
        if (compare_values(tmp_requirement->getValueForKey(in_key), in_value) == 0)
            return tmp_requirement;

        if (in_recursive)
        {
            tmp_found_requirement = findRequirementWithValueForKey(tmp_requirement->_m_childs, in_value, in_key, true);
            if (tmp_found_requirement != NULL)
                return tmp_found_requirement;
        }
    }

    return NULL;
}


void RequirementHierarchy::writeXml(QXmlStreamWriter & in_xml_writer)
{
    RequirementContent	    tmp_requirement_content = RequirementContent();

    in_xml_writer.writeStartElement("requirement");
    in_xml_writer.writeAttribute("id", getIdentifier());
    in_xml_writer.writeAttribute("requirementContentId", getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID));

    if (_m_childs.count() > 0)
    {
        in_xml_writer.writeStartElement("requirements");
        foreach(RequirementHierarchy *tmp_requirement, _m_childs)
        {
            tmp_requirement->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
    }

    in_xml_writer.writeEndElement();
}


bool RequirementHierarchy::readXml(QXmlStreamReader & in_xml_reader)
{
    RequirementHierarchy	*tmp_requirement = NULL;

    QString			tmp_id = in_xml_reader.attributes().value("id").toString();
    QString			tmp_test_content_id = in_xml_reader.attributes().value("requirementContentId").toString();

    const char			*tmp_previous_requirement_id = NULL;

    setValueForKey(tmp_id.toStdString().c_str(), REQUIREMENTS_HIERARCHY_REQUIREMENT_ID);
    setValueForKey(tmp_test_content_id.toStdString().c_str(), REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID);

    while (in_xml_reader.readNextStartElement())
    {
        if (in_xml_reader.name() == "requirements")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "requirement")
                {
                    tmp_requirement = new RequirementHierarchy(this);
                    tmp_requirement->setValueForKey(tmp_id.toStdString().c_str(), REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);
                    tmp_requirement->setValueForKey(tmp_previous_requirement_id, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
                    tmp_requirement->readXml(in_xml_reader);
                    tmp_previous_requirement_id = tmp_requirement->getIdentifier();
                    _m_childs.append(tmp_requirement);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }
        else
        {
            LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
            in_xml_reader.skipCurrentElement();
        }
    }

    return true;
}


int RequirementHierarchy::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int			tmp_result = NOERR;
    QString		tmp_requirement_id = QString(getIdentifier());
    const char		*tmp_requirement_content_id = getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID);
    const char		*tmp_parent_requirement_id = getValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);
    const char		*tmp_previous_requirement_id = getValueForKey(REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
    Requirement		*tmp_requirement_record = NULL;

    if (is_empty_string(getOriginalValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_ID)) == TRUE)
    {
        setValueForKey(NULL, REQUIREMENTS_HIERARCHY_REQUIREMENT_ID);

        if (_m_project_version != NULL)
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), REQUIREMENTS_HIERARCHY_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), REQUIREMENTS_HIERARCHY_VERSION);
        }

        if (is_empty_string(tmp_parent_requirement_id) == FALSE)
            setValueForKey(in_xml_datas.getNewRequirementHierarchyIdentifier(_m_project_version, tmp_parent_requirement_id), REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID);

        if (is_empty_string(tmp_previous_requirement_id) == FALSE)
            setValueForKey(in_xml_datas.getNewRequirementHierarchyIdentifier(_m_project_version, tmp_previous_requirement_id), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

        setValueForKey(in_xml_datas.getNewRequirementContentIdentifier(tmp_requirement_content_id), REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID);

        tmp_requirement_record = new Requirement();
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID), REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID);
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID), REQUIREMENTS_TABLE_PREVIOUS_REQUIREMENT_ID);
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID), REQUIREMENTS_TABLE_PROJECT_ID);
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID), REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID);
        tmp_requirement_record->setValueForKey(getValueForKey(REQUIREMENTS_HIERARCHY_VERSION), REQUIREMENTS_TABLE_VERSION);

        tmp_result = tmp_requirement_record->insertRecord();
        if (tmp_result == NOERR)
        {
            in_xml_datas.m_requirements_dict.insert(tmp_requirement_id, this);

            setValueForKey(tmp_requirement_record->getIdentifier(), REQUIREMENTS_HIERARCHY_REQUIREMENT_ID);
            cloneColumns();

            foreach(RequirementHierarchy *tmp_requirement, _m_childs)
            {
                tmp_requirement->saveFromXmlProjectDatas(in_xml_datas);
            }
        }
    }

    return tmp_result;
}


QList<TestHierarchy*> RequirementHierarchy::dependantsTests()
{
    net_session         *tmp_session = CLIENT_SESSION;
    char                *tmp_query = tmp_session->m_last_query;
    char                ***tmp_query_results = NULL;
    unsigned long       tmp_rows_count = 0;
    unsigned long       tmp_columns_count = 0;

    TestHierarchy       *tmp_test = NULL;
    QList<TestHierarchy*>     tmp_final_tests_list;

    tmp_query += sprintf(tmp_query, "SELECT %s FROM %s WHERE %s=%s;"
                         , TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID
                         , TESTS_REQUIREMENTS_TABLE_SIG
                         , TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID
                         , getValueForKey(REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID));

    tmp_query_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
    if (tmp_query_results != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_rows_count; tmp_index++)
        {
            tmp_test = (TestHierarchy*)_m_project_version->findItemWithValueForKey(tmp_query_results[tmp_index][0], TESTS_HIERARCHY_TEST_CONTENT_ID, TESTS_HIERARCHY_SIG_ID);
            if (tmp_test != NULL)
            {
                while (tmp_test->original() != NULL && tmp_test->parent() != _m_project_version)
                {
                    tmp_test = (TestHierarchy*)tmp_test->parent();
                }

                if (tmp_final_tests_list.indexOf(tmp_test) < 0)
                    tmp_final_tests_list.append(tmp_test);

                foreach (TestHierarchy *tmp_copy, tmp_test->links())
                {
                    while (tmp_copy->original() != NULL && tmp_copy->parent() != _m_project_version)
                    {
                        tmp_copy = (TestHierarchy*)tmp_copy->parent();
                    }

                    if (tmp_final_tests_list.indexOf(tmp_copy) < 0)
                        tmp_final_tests_list.append(tmp_copy);
                }

            }
        }
        cl_free_rows_columns_array(&tmp_query_results, tmp_rows_count, tmp_columns_count);
    }

    return tmp_final_tests_list;
}


void RequirementHierarchy::setDataFromRequirementContent(RequirementContent *in_requirement_content)
{
    if (in_requirement_content != NULL)
    {
        setValueForKey(in_requirement_content->getIdentifier(), REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME), REQUIREMENTS_HIERARCHY_SHORT_NAME);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID), REQUIREMENTS_HIERARCHY_CATEGORY_ID);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL), REQUIREMENTS_HIERARCHY_PRIORITY_LEVEL);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION), REQUIREMENTS_HIERARCHY_CONTENT_VERSION);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_STATUS), REQUIREMENTS_HIERARCHY_STATUS);
        setValueForKey(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID), REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID);
    }
}


QList<CustomRequirementField*> RequirementHierarchy::loadCustomFields()
{
    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", CUSTOM_REQUIREMENT_FIELDS_TABLE_REQUIREMENT_CONTENT_ID, getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID));
    return CustomRequirementField::loadRecordsList(CLIENT_SESSION->m_where_clause_buffer);
}
