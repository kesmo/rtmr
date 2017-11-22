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

#ifndef XMLPROJECTDATAS_H_
#define XMLPROJECTDATAS_H_

#include <QHash>

class ProjectVersion;

class TestContent;
class TestHierarchy;

class RequirementContent;
class RequirementHierarchy;

class TestRequirement;
class Action;
class AutomatedAction;
class AutomatedActionValidation;

class XmlProjectDatas {
public:
	XmlProjectDatas();
	virtual ~XmlProjectDatas();

	const char* getNewTestContentIdentifier(const char *in_identifier);
	QString getNewTestContentOriginalIdentifier(TestContent *in_test_content, const char *in_original_identifier);
	const char* getNewRequirementContentIdentifier(const char *in_identifier);
	QString getNewRequirementContentOriginalIdentifier(const char *in_original_identifier);
	const char* getNewTestHierarchyIdentifier(ProjectVersion* in_project_version, const char *in_identifier);
	const char* getNewRequirementHierarchyIdentifier(ProjectVersion* in_project_version, const char *in_identifier);

	QHash<QString, TestHierarchy*>		    m_tests_dict;
	QHash<QString, RequirementHierarchy*>	m_requirements_dict;

	QHash<QString, TestContent*>		    m_tests_contents_dict;
	QHash<QString, QString>			    m_originals_tests_contents_dict;
	QHash<QString, RequirementContent*>	    m_requirements_contents_dict;
	QHash<QString, QString>			    m_originals_requirements_contents_dict;

	QHash<QString, QList<Action*> >		    m_actions_dict;
    QHash<QString, QList<AutomatedAction*> >		    m_automated_actions_dict;
    QHash<QString, QList<TestRequirement*> >    m_tests_requirements_dict;

    QList<TestContent*>				    m_tests_contents_list;
    QList<RequirementContent*>			    m_requirements_contents_list;
    QList<ProjectVersion*>			    m_versions_list;

};

#endif /* XMLPROJECTDATAS_H_ */
