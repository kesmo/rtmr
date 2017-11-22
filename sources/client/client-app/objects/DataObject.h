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


#ifndef DATAOBJECT_H_
#define DATAOBJECT_H_

#include "Comparable.h"
#include <QString>
#include <QDateTime>

class DataObject : public Comparable
{
public:

        enum DataType
        {
            String = 0,
            Number = 1,
            Date = 2,
            Other = 99
        };

	DataObject(DataObject::DataType in_data_type);
	DataObject(const DataObject &in_object);
	virtual ~DataObject();

	bool isNumber();
        bool isDate();

	virtual QString toString() = 0;
	virtual double doubleValue() = 0;
        virtual DataObject &operator=(const DataObject &/*in_object*/)
	{
		return *this;
	}

	/**
	 * Comparer deux objets
	 */
	static int compare(DataObject *in_first_object, DataObject *in_sec_object)
	{
	    if (in_first_object != NULL)
	    {
	        if (in_sec_object == NULL)
	        	return -1;
	    }
	    else if (in_sec_object == NULL)
	        return 0;
	    else
	        return 1;

		return in_first_object->compareTo(*in_sec_object);
	}


	/**
	 * Définir l'égalité de deux objets
	 * @param in_first_object
	 * @param in_sec_object
	 * @return
	 */
	static bool areEquals(DataObject *in_first_object, DataObject *in_sec_object)
	{
		return (in_first_object == in_sec_object
			|| (in_first_object != NULL && in_sec_object != NULL && in_first_object->equals(*in_sec_object)));
	}


private:
        DataObject::DataType    _m_data_type;

};

class Double : public DataObject
{
private:
        double _m_value;

public:
	Double(double in_value);
	~Double();


	double doubleValue();
	QString toString();
    DataObject &operator=(const Double &in_object);
    DataObject &operator+=(double in_value);

	int compareTo(const Comparable &in_objet) const;
	bool equals(const Comparable &in_objet) const;

	bool operator==(const Comparable &in_objet) const;
	bool operator>(const Comparable &in_objet) const;
	bool operator<(const Comparable &in_objet) const;
	bool operator>=(const Comparable &in_objet) const;
	bool operator<=(const Comparable &in_objet) const;
};

class Integer : public DataObject
{
private:
	int _m_value;

public:
	Integer(int in_value);
	~Integer();


	int intValue();
	double doubleValue();
	QString toString();
    DataObject &operator=(const Integer &in_object);

	int compareTo(const Comparable &in_objet) const;
	bool equals(const Comparable &in_objet) const;

	bool operator==(const Comparable &in_objet) const;
	bool operator>(const Comparable &in_objet) const;
	bool operator<(const Comparable &in_objet) const;
	bool operator>=(const Comparable &in_objet) const;
	bool operator<=(const Comparable &in_objet) const;
};


class Date : public DataObject
{
private:
        QDateTime _m_value;

public:
	Date();
	Date(QDateTime in_date);
	Date(uint in_time);
	~Date();

	QDateTime dateTime(){return _m_value;}
	QString toString();
	double doubleValue();
    DataObject &operator=(const Date &in_object);

	int compareTo(const Comparable &in_objet) const;
	bool equals(const Comparable &in_objet) const;

	bool operator==(const Comparable &in_objet) const;
	bool operator>(const Comparable &in_objet) const;
	bool operator<(const Comparable &in_objet) const;
	bool operator>=(const Comparable &in_objet) const;
	bool operator<=(const Comparable &in_objet) const;

};


class String : public DataObject
{
private:
	QString _m_value;

public:
	String(QString in_str);
	~String();

	QString toString();
	double doubleValue();
    DataObject &operator=(const String &in_object);

	int compareTo(const Comparable &in_objet) const;
	bool equals(const Comparable &in_objet) const;

	bool operator==(const Comparable &in_objet) const;
	bool operator>(const Comparable &in_objet) const;
	bool operator<(const Comparable &in_objet) const;
	bool operator>=(const Comparable &in_objet) const;
	bool operator<=(const Comparable &in_objet) const;

};

#endif /* DATAOBJECT_H_ */
