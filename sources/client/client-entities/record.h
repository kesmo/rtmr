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

#ifndef RECORD_H
#define RECORD_H

#include <QVariant>
#include <QTextStream>
#include <QTextCodec>

#include <string.h>
#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <stdlib.h>
#include "netcommon.h"
#include "client.h"
#include "utilities.h"
#include "entities.h"

class Record
{
    private:
        /* Tableau des champs */
        char    **_m_columns_record;
        char    **_m_originals_columns_record;

    protected:

        static net_session *CLIENT_SESSION;

        int     _m_record_status;

        Record(char **in_columns_record);

        int compareField(Record *in_record, char* in_field_name) const;

    public:

        enum SearchComparison
        {
            EqualTo = 0,
            LowerThan,
            UpperThan,
            LowerOrEqualTo,
            UpperOrEqualTo
        };

        static void init(net_session *in_session){
            CLIENT_SESSION = in_session;
        }

        Record(const entity_def* in_entity_def);

        virtual ~Record() = 0;

        virtual void destroy();

        const char* getIdentifier() const;

        void cloneColumns();

        int setValueForKey(const char* in_new_value, const char* in_key);

        int setValueForKeyAtIndex(const char* in_new_value, unsigned int in_key_index);

        const char* getValueForKey(const char* in_key) const;

        const char* getOriginalValueForKey(const char* in_key) const;

        const char* getValueForKeyAtIndex(unsigned int in_key_index) const;

        virtual const entity_def* getEntityDef() const = 0;

        int getEntityDefSignatureId() const;

        int loadRecord(const char *in_primary_key_value);

        int deleteRecord();

        static int deleteRecords(int in_signature_id, char* in_where_clause);

        int saveRecord(Record *in_original_record);

        int saveRecord();

        int insertRecord();

        template < class T >
        static const char* matchingValueInRecordsList(const QList<T*>& in_records, const char *in_discriminant_key, const char *in_discriminant_value, const char *in_search_key)
        {
            const char        *tmp_discrimant_key_value = NULL;

            foreach (T* tmp_record, in_records)
            {
                tmp_discrimant_key_value = tmp_record->getValueForKey(in_discriminant_key);
                if (compare_values(tmp_discrimant_key_value, in_discriminant_value) == 0)
                {
                    return tmp_record->getValueForKey(in_search_key);
                }
            }

            return NULL;
        }

        template < class T >
        static int indexOfMatchingValueInRecordsList(const QList<T*>& in_records, const char *in_discriminant_key, const char *in_discriminant_value)
        {
            const char        *tmp_discrimant_key_value = NULL;
            int	    tmp_index = 0;

            foreach (T* tmp_record, in_records)
            {
                tmp_discrimant_key_value = tmp_record->getValueForKey(in_discriminant_key);
                if (compare_values(tmp_discrimant_key_value, in_discriminant_value) == 0)
                {
                    return tmp_index;
                }
                tmp_index++;
            }

            return -1;
        }



        /**
     * Charger plusieurs enregistrements de la base de donnees
     * @param in_record_class
     * @param in_signature
     * @param in_where_clause
     * @return
     */
        template < class T >
        static T** loadRecords(net_session *in_session, const entity_def *in_entity_def, const char* in_where_clause, const char* in_order_by_clause, unsigned long *in_out_records_count = NULL, net_callback_fct *in_callback = NULL)
        {
            char***         tmp_rows = NULL;
            T**             tmp_records = NULL;

            unsigned long   tmp_records_count = 0;


            tmp_rows = cl_load_records(in_session, in_entity_def->m_entity_signature_id , in_where_clause, in_order_by_clause, &tmp_records_count, in_callback);
            if (tmp_rows != NULL)
            {
                tmp_records = new T*[tmp_records_count];

                for (unsigned long tmp_index = 0; tmp_index < tmp_records_count; tmp_index++)
                {
                    tmp_records[tmp_index] = new T;
                    for (unsigned long tmp_column_index = 0; tmp_column_index < in_entity_def->m_entity_columns_count; tmp_column_index++)
                    {
                        tmp_records[tmp_index]->_m_columns_record[tmp_column_index] = tmp_rows[tmp_index][tmp_column_index];
                    }
                    tmp_records[tmp_index]->cloneColumns();
                }

                cl_free_rows_columns_array(&tmp_rows, tmp_records_count, 0);
            }

            if (in_out_records_count != NULL)
                *in_out_records_count = tmp_records_count;

            return tmp_records;
        }


        template < class T >
        static T** freeRecords(T** in_records, unsigned long in_records_count)
        {
            if (in_records != NULL)
            {
                for (unsigned long tmp_index = 0; tmp_index < in_records_count; tmp_index++)
                {
                    if (in_records[tmp_index] != NULL)
                        delete in_records[tmp_index];
                }
                delete in_records;
            }

            return NULL;
        }


        int compareTo(Record *in_record) const;

        int compareTo(Record *in_record, char* in_column_name) const;

        /**
     * Cloner l'enregistrement
     */
        template< class T>
        T* clone(bool in_clone_originals_columns = true)
        {
            T *tmp_new_record = new T;
            int tmp_columns_count = getEntityDef()->m_entity_columns_count;

            if (_m_record_status == RECORD_STATUS_OWN_LOCK)
                tmp_new_record->_m_record_status = RECORD_STATUS_MODIFIABLE;
            else
                tmp_new_record->_m_record_status = _m_record_status;

            for (int tmp_index = 0; tmp_index < tmp_columns_count; tmp_index++)
            {
                if (_m_columns_record[tmp_index] != NULL)
                {
                    tmp_new_record->_m_columns_record[tmp_index] = cl_alloc_str(strlen(_m_columns_record[tmp_index]));
                    strcpy(tmp_new_record->_m_columns_record[tmp_index], _m_columns_record[tmp_index]);
                }
                else
                    tmp_new_record->_m_columns_record[tmp_index] = NULL;
            }

            if (in_clone_originals_columns)
                tmp_new_record->cloneColumns();

            return tmp_new_record;
        }


        template<class T>
        static QList < T* > orderedRecords(T **in_records, unsigned long in_records_count, const char *in_previous_key)
        {
            unsigned long       tmp_records_index = 0;
            T                   *tmp_record = NULL;
            QList < T* >        tmp_records_list;

            const char                  *tmp_previous_id = NULL;


            if (in_records != NULL && in_records_count > 0)
            {
                do
                {
                    for (tmp_records_index = 0; tmp_records_index < in_records_count; tmp_records_index++)
                    {
                        tmp_record = in_records[tmp_records_index];
                        if (tmp_record != NULL
                        && compare_values(tmp_previous_id, tmp_record->getValueForKey(in_previous_key)) == 0)
                        {
                            tmp_records_list.append(tmp_record);
                            tmp_previous_id = tmp_record->getIdentifier();

                            break;
                        }
                    }
                }
                while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_records_index >= in_records_count));

                for (tmp_records_index = 0; tmp_records_index < in_records_count; tmp_records_index++)
                {
                    tmp_record = in_records[tmp_records_index];
                    if (tmp_record != NULL && tmp_records_list.indexOf(tmp_record) < 0)
                    {
                        tmp_records_list.append(tmp_record);
                    }
                }

            }

            return tmp_records_list;
        }


        template<class T>
        static QList<T*> readDataFromDevice(QTextStream& in_text_stream, QList<char>& in_fields_sep, QList<char>& in_records_sep, QList<const char*>& in_field_names)
        {
            QChar	    tmp_char;
            char	    tmp_text_separator = '\"';

            int tmp_field_char_index = 0;
            int tmp_record_char_index = 0;

            int		    tmp_column_number = 0;
            int		    tmp_row_number = 0;

            QByteArray	    tmp_str;
            QByteArray	    tmp_field_sep_str;
            QByteArray	    tmp_record_sep_str;

            bool	    tmp_first_char_of_field = true;
            bool	    tmp_first_char_is_quote = false;
            bool	    tmp_previous_char_is_quote = false;

            QList<T*>	    tmp_records_list;
            T		    *tmp_record = NULL;


            if (!in_text_stream.atEnd())
                in_text_stream >> tmp_char;

            while (!in_text_stream.atEnd())
            {
                // Gestion des double-quotes
                if (tmp_char==tmp_text_separator)
                {
                    if (tmp_first_char_of_field)
                    {
                        tmp_first_char_is_quote = true;
                    }
                    else
                    {
                        if (tmp_previous_char_is_quote)
                        {
                            tmp_str += tmp_char;
                            tmp_previous_char_is_quote = false;
                        }
                        else
                            tmp_previous_char_is_quote = true;
                    }

                    tmp_first_char_of_field = false;
                    if (!in_text_stream.atEnd())
                        in_text_stream >> tmp_char;
                }
                // Separateur de champs
                else if (tmp_char == in_fields_sep[0])
                {
                    tmp_first_char_of_field = false;
                    tmp_field_sep_str = QByteArray();

                    while (!in_text_stream.atEnd() && tmp_field_char_index < in_fields_sep.length() && tmp_char == in_fields_sep[tmp_field_char_index])
                    {
                        tmp_field_sep_str += tmp_char;
                        tmp_field_char_index++;
                        if (!in_text_stream.atEnd())
                            in_text_stream >> tmp_char;
                    }

                    if (tmp_field_char_index < in_fields_sep.length())
                    {
                        tmp_str += tmp_field_sep_str;
                    }
                    // Enregistrement du champs
                    else
                    {
                        if (!tmp_first_char_is_quote || (tmp_first_char_is_quote && tmp_previous_char_is_quote))
                        {
                            if (tmp_column_number == 0)
                            {
                                tmp_record = new T();
                                tmp_records_list.append(tmp_record);
                            }

                            if (tmp_column_number < in_field_names.length())
                            {
                                tmp_record->setValueForKey(tmp_str, in_field_names[tmp_column_number]);
                            }

                            tmp_column_number++;

                            tmp_str.clear();

                            tmp_first_char_is_quote = false;
                            tmp_first_char_of_field = true;
                        }
                        else
                        {
                            tmp_str += tmp_field_sep_str;
                        }
                    }

                    tmp_previous_char_is_quote = false;
                    tmp_field_char_index = 0;
                }
                // Separateur d'enregistrements
                else if (tmp_char == in_records_sep[0])
                {
                    tmp_first_char_of_field = false;
                    tmp_record_sep_str = QByteArray();

                    while (!in_text_stream.atEnd() && tmp_record_char_index < in_records_sep.length() && tmp_char == in_records_sep[tmp_record_char_index])
                    {
                        tmp_record_sep_str += tmp_char;
                        tmp_record_char_index++;
                        if (!in_text_stream.atEnd())
                            in_text_stream >> tmp_char;
                    }

                    if (tmp_record_char_index < in_records_sep.length())
                    {
                        tmp_str += tmp_record_sep_str;
                    }
                    // Enregistrement du champs
                    else
                    {
                        if (!tmp_first_char_is_quote || (tmp_first_char_is_quote && tmp_previous_char_is_quote))
                        {
                            if (tmp_column_number < in_field_names.length())
                            {
                                if (tmp_record == NULL)
                                {
                                    tmp_record = new T();
                                    tmp_records_list.append(tmp_record);
                                }
                                tmp_record->setValueForKey(tmp_str, in_field_names[tmp_column_number]);
                            }
                            tmp_row_number++;

                            tmp_column_number = 0;
                            tmp_str.clear();

                            tmp_first_char_is_quote = false;
                            tmp_first_char_of_field = true;
                        }
                        else
                        {
                            tmp_str += tmp_record_sep_str;
                        }
                    }

                    tmp_previous_char_is_quote = false;
                    tmp_record_char_index = 0;
                }
                else
                {
                    tmp_previous_char_is_quote = false;
                    tmp_first_char_of_field = false;
                    tmp_str += tmp_char;
                    if (!in_text_stream.atEnd())
                        in_text_stream >> tmp_char;
                }
            }

            if (tmp_str.isEmpty() == false)
            {
                if (tmp_column_number < in_field_names.length())
                {
                    if (tmp_record == NULL)
                    {
                        tmp_record = new T();
                        tmp_records_list.append(tmp_record);
                    }

                    tmp_record->setValueForKey(tmp_str, in_field_names[tmp_column_number]);
                }
            }

            return tmp_records_list;
        }

        int lockRecordStatus() const;

        int setRecordStatus(int in_record_status);

        int lockRecord(bool in_check_sync);

        int unlockRecord();

        QString serialize() const;

        void deserialize(const QString & in_serialized_record);

        virtual bool hasChangedValues() const;

};



template <class T> inline QList<Record*> castToRecordsList(const QList<T*>& in_records_list)
{
    QList<Record*>   tmp_list;
    Record*          tmp_cast_record = NULL;

    foreach (T *tmp_record, in_records_list)
    {
        tmp_cast_record = dynamic_cast<Record*>(tmp_record);
        if (tmp_cast_record != NULL)
            tmp_list << tmp_cast_record;
    }

    return tmp_list;
}


template <class T>
class Entity : public Record
{
    public:
        Entity() : Record(T::getEntityDefinition()){}
        ~Entity(){destroy();}
        virtual const entity_def* getEntityDef() const {return T::getEntityDefinition();}

        static T** loadRecords(const char* in_where_clause = NULL, const char* in_order_by_clause = NULL, unsigned long *in_out_records_count = NULL, net_callback_fct *in_callback = NULL)
        {
            return Record::loadRecords<T>(CLIENT_SESSION, T::getEntityDefinition(), in_where_clause, in_order_by_clause, in_out_records_count, in_callback);
        }

        static QList<T*> loadRecordsList(const char* in_where_clause = NULL, const char* in_order_by_clause = NULL, unsigned long *in_out_records_count = NULL, net_callback_fct *in_callback = NULL)
        {
            QList<T*>   tmp_return_list;
            unsigned long tmp_records_count = 0;
            T** tmp_records_array = loadRecords(in_where_clause, in_order_by_clause, &tmp_records_count, in_callback);
            if (tmp_records_array != NULL)
            {
                for (unsigned long tmp_index = 0; tmp_index < tmp_records_count; tmp_index++)
                {
                    tmp_return_list.append(tmp_records_array[tmp_index]);
                }
                tmp_records_array = Record::freeRecords<T>(tmp_records_array, 0);
            }

            if (in_out_records_count)
                (*in_out_records_count) = tmp_records_count;

            return tmp_return_list;
        }

        static int indexForKey(const char *in_key)
        {
            for (unsigned int tmp_index = 0; tmp_index < T::getEntityDefinition()->m_entity_columns_count; tmp_index++)
            {
                if (compare_values(T::getEntityDefinition()->m_entity_columns_names[tmp_index], in_key) == 0)
                    return tmp_index;
            }

            return -1;
        }

};

#endif // RECORD_H
