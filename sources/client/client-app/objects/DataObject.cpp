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

#include "DataObject.h"

/**
* Constructeur
* @param in_object
*/
DataObject::DataObject(DataObject::DataType in_data_type) : Comparable()
{
        _m_data_type = in_data_type;
}


DataObject::DataObject(const DataObject &in_object) : Comparable()
{
        _m_data_type = in_object._m_data_type;
}

DataObject::~DataObject()
{
}

bool DataObject::isNumber()
{
        return _m_data_type == Number;
}


bool DataObject::isDate()
{
        return _m_data_type == Date;
}

/**********************************************
 * Double
 * *******************************************/
Double::Double(double in_value) : DataObject(DataObject::Number)
{
	_m_value = in_value;
}

Double::~Double()
{
}

DataObject &Double::operator=(const Double &in_object)
{
	if (this != &in_object)
        _m_value = in_object._m_value;

	return *this;
}


DataObject &Double::operator+=(double in_value)
{
    _m_value += in_value;
    return *this;
}


double Double::doubleValue()
{
	return _m_value;
}

QString Double::toString()
{
	return QString::number(_m_value);
}


int Double::compareTo(const Comparable &in_objet) const
{
	if (_m_value > ((Double*)&in_objet)->_m_value)
		return 1;
	else if (_m_value < ((Double*)&in_objet)->_m_value)
		return -1;
	else
		return 0;
}

bool Double::equals(const Comparable &in_objet) const
{
	return _m_value == ((Double*)&in_objet)->_m_value;
}

bool Double::operator==(const Comparable &in_objet) const
{
	return _m_value == ((Double*)&in_objet)->_m_value;
}

bool Double::operator>(const Comparable &in_objet) const
{
	return _m_value > ((Double*)&in_objet)->_m_value;
}

bool Double::operator<(const Comparable &in_objet) const
{
	return _m_value < ((Double*)&in_objet)->_m_value;
}

bool Double::operator>=(const Comparable &in_objet) const
{
	return _m_value >= ((Double*)&in_objet)->_m_value;
}

bool Double::operator<=(const Comparable &in_objet) const
{
	return _m_value <= ((Double*)&in_objet)->_m_value;
}

/**********************************************
 * Integer
 * *******************************************/
Integer::Integer(int in_value) : DataObject(DataObject::Number)
{
	_m_value = in_value;
}

Integer::~Integer()
{
}

DataObject &Integer::operator=(const Integer &in_object)
{
	if (this != &in_object)
        _m_value = in_object._m_value;

	return *this;
}


int Integer::intValue()
{
	return _m_value;
}


double Integer::doubleValue()
{
	return _m_value;
}

QString Integer::toString()
{
	return QString::number(_m_value);
}


int Integer::compareTo(const Comparable &in_objet) const
{
	if (_m_value > ((Integer*)&in_objet)->_m_value)
		return 1;
	else if (_m_value < ((Integer*)&in_objet)->_m_value)
		return -1;
	else
		return 0;
}

bool Integer::equals(const Comparable &in_objet) const
{
	return _m_value == ((Integer*)&in_objet)->_m_value;
}

bool Integer::operator==(const Comparable &in_objet) const
{
	return _m_value == ((Integer*)&in_objet)->_m_value;
}

bool Integer::operator>(const Comparable &in_objet) const
{
	return _m_value > ((Integer*)&in_objet)->_m_value;
}

bool Integer::operator<(const Comparable &in_objet) const
{
	return _m_value < ((Integer*)&in_objet)->_m_value;
}

bool Integer::operator>=(const Comparable &in_objet) const
{
	return _m_value >= ((Integer*)&in_objet)->_m_value;
}

bool Integer::operator<=(const Comparable &in_objet) const
{
	return _m_value <= ((Integer*)&in_objet)->_m_value;
}

/**********************************************
 * Date
 * *******************************************/
Date::Date() : DataObject(DataObject::Date)
{
	_m_value = QDateTime();
}


Date::Date(QDateTime in_date) : DataObject(DataObject::Date)
{
	_m_value = in_date;
}

Date::Date(uint in_time) : DataObject(DataObject::Date)
{
	_m_value = QDateTime();
	_m_value.setTime_t(in_time);
}

Date::~Date()
{
}

DataObject &Date::operator=(const Date &in_object)
{
	if (this != &in_object)
        _m_value = in_object._m_value;

	return *this;
}

double Date::doubleValue()
{
	return _m_value.toTime_t();
}

QString Date::toString()
{
	return _m_value.toString();
}


int Date::compareTo(const Comparable &in_objet) const
{
	if (_m_value > ((Date*)&in_objet)->_m_value)
		return 1;
	else if (_m_value < ((Date*)&in_objet)->_m_value)
		return -1;
	else
		return 0;
}

bool Date::equals(const Comparable &in_objet) const
{
	return _m_value == ((Date*)&in_objet)->_m_value;
}

bool Date::operator==(const Comparable &in_objet) const
{
	return _m_value == ((Date*)&in_objet)->_m_value;
}

bool Date::operator>(const Comparable &in_objet) const
{
	return _m_value > ((Date*)&in_objet)->_m_value;
}

bool Date::operator<(const Comparable &in_objet) const
{
	return _m_value < ((Date*)&in_objet)->_m_value;
}

bool Date::operator>=(const Comparable &in_objet) const
{
	return _m_value >= ((Date*)&in_objet)->_m_value;
}

bool Date::operator<=(const Comparable &in_objet) const
{
	return _m_value <= ((Date*)&in_objet)->_m_value;
}

/**********************************************
 * String
 * *******************************************/
String::String(QString in_date) : DataObject(DataObject::String)
{
	_m_value = in_date;
}

String::~String()
{
}

DataObject &String::operator=(const String &in_object)
{
	if (this != &in_object)
        _m_value = in_object._m_value;

	return *this;
}

double String::doubleValue()
{
	return _m_value.toDouble();
}

QString String::toString()
{
	return _m_value;
}


int String::compareTo(const Comparable &in_objet) const
{
	if (_m_value > ((String*)&in_objet)->_m_value)
		return 1;
	else if (_m_value < ((String*)&in_objet)->_m_value)
		return -1;
	else
		return 0;
}

bool String::equals(const Comparable &in_objet) const
{
	return _m_value.compare(((String*)&in_objet)->_m_value) == 0;
}

bool String::operator==(const Comparable &in_objet) const
{
	return _m_value == ((String*)&in_objet)->_m_value;
}

bool String::operator>(const Comparable &in_objet) const
{
	return _m_value > ((String*)&in_objet)->_m_value;
}

bool String::operator<(const Comparable &in_objet) const
{
	return _m_value < ((String*)&in_objet)->_m_value;
}

bool String::operator>=(const Comparable &in_objet) const
{
	return _m_value >= ((String*)&in_objet)->_m_value;
}

bool String::operator<=(const Comparable &in_objet) const
{
	return _m_value <= ((String*)&in_objet)->_m_value;
}

