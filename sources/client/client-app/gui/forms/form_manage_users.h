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

#ifndef FORM_MANAGE_USERS_H
#define FORM_MANAGE_USERS_H

#include <QList>
#include <QtGui/QDialog>

namespace Ui {
    class Form_Manage_Users;
}


class DBUser {
private:
	bool	_m_new_user;
        char	*_m_username;
        char	*_m_password;
	bool	_m_admin_role;
	bool	_m_writer_role;
	bool	_m_reader_role;

public:
	DBUser(const char *in_username, bool in_admin_role, bool in_writer_role, bool in_reader_role)
	{
		_m_new_user = false;
                _m_username = (char*)malloc(strlen(in_username) + 1);
                strcpy(_m_username, in_username);
                _m_password = NULL;
		_m_admin_role = in_admin_role;
		_m_writer_role = in_writer_role;
		_m_reader_role = in_reader_role;
	}


	DBUser(const char *in_username, const char *in_password)
	{
		_m_new_user = true;
                _m_username = (char*)malloc(strlen(in_username) + 1);
                strcpy(_m_username, in_username);
                _m_password = (char*)malloc(strlen(in_password) + 1);
                strcpy(_m_password, in_password);
                _m_admin_role = false;
		_m_writer_role = false;
		_m_reader_role = true;
	}

        ~DBUser(){free(_m_username);free(_m_password);}

	const char* username(){return _m_username;}
	const char* password(){return _m_password;}
	bool isAdmin(){return _m_admin_role;}
	bool isWriter(){return _m_writer_role;}
	bool isReader(){return _m_reader_role;}
	bool isNewUser(){return _m_new_user;}
};


class Form_Manage_Users : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Form_Manage_Users)
public:
    explicit Form_Manage_Users(QWidget *parent = 0);
    virtual ~Form_Manage_Users();

    void loadUsers();

public Q_SLOTS:
	void accept();
	void addUser();
    void changePassword();
    void manageUserGrants();
	void addUser(DBUser *in_user);
	void removeSelectedUsers();
	void userSelectionChanged();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Form_Manage_Users	*_m_ui;

    QList<DBUser*>			_m_users_list;
    QList<DBUser*>			_m_removed_users_list;

    void setUserAtIndex(DBUser *in_user, int in_index);
};

#endif // FORM_MANAGE_USERS_H
