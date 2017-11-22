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

#include "record.h"
#include "utilities.h"
#include "constants.h"
#include "client.h"
#include "entities.h"
#include "netcommon.h"

#include <string.h>

#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif


net_session *Record::CLIENT_SESSION = NULL;

/**
 * Constructeur
 */
Record::Record(const entity_def* in_entity_def)
{
  _m_columns_record = cl_alloc_columns_array(in_entity_def->m_entity_columns_count);
  _m_originals_columns_record = NULL;
  _m_record_status = RECORD_STATUS_MODIFIABLE;
}


/**
 * Constructeur
 */
Record::Record(char **in_columns_record)
{
  _m_columns_record = in_columns_record;
  _m_record_status = RECORD_STATUS_MODIFIABLE;
}



/**
  Destructeur
**/
Record::~Record()
{
}


void Record::cloneColumns()
{
  int		tmp_columns_count = getEntityDef()->m_entity_columns_count;

  cl_free_columns_array(&_m_originals_columns_record, tmp_columns_count);
  _m_originals_columns_record = cl_alloc_columns_array(tmp_columns_count);

  for (int tmp_index = 0; tmp_index < tmp_columns_count; tmp_index++)
    {
      if (_m_columns_record[tmp_index] != NULL)
        {
          _m_originals_columns_record[tmp_index] = cl_alloc_str(strlen(_m_columns_record[tmp_index]));
          strcpy(_m_originals_columns_record[tmp_index], _m_columns_record[tmp_index]);
        }
      else
        _m_originals_columns_record[tmp_index] = NULL;
    }
}


void Record::destroy()
{
  const int	tmp_columns_count = getEntityDef()->m_entity_columns_count;

  unlockRecord();

  if (_m_columns_record != NULL)
    cl_free_columns_array(&_m_columns_record, tmp_columns_count);

  if (_m_originals_columns_record != NULL)
    cl_free_columns_array(&_m_originals_columns_record, tmp_columns_count);
}

/**
 * Renvoie la valeur de la cle primaire
 * @return
 */
const char* Record::getIdentifier() const
{
  if (_m_columns_record != NULL)
    return _m_columns_record[0];

  return NULL;
}


int Record::setValueForKey(const char* in_new_value, const char* in_key)
{
  unsigned int			tmp_value_length = 0;
  const char**	tmp_entity_def_columns_names = getEntityDef()->m_entity_columns_names;

  if (is_empty_string(in_key) == false)
    {
      for (unsigned int tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
        {
          if (is_empty_string(tmp_entity_def_columns_names[tmp_index]) == false && strcmp(tmp_entity_def_columns_names[tmp_index], in_key) == 0)
            {
              if (in_new_value != NULL)
                {
                  tmp_value_length = strlen(in_new_value);

                  if (_m_columns_record[tmp_index] != NULL)
                    cl_free_str(&_m_columns_record[tmp_index]);

                  _m_columns_record[tmp_index] = cl_alloc_str(tmp_value_length);
                  strcpy(_m_columns_record[tmp_index], in_new_value);
                }
              else
                {
                  if (_m_columns_record[tmp_index] != NULL){
                      cl_free_str(&_m_columns_record[tmp_index]);
                    }
                }

              return NOERR;
            }
        }

      LOG_ERROR(CLIENT_SESSION, "Cannot SET value for unknow column named '%s' in entity '%s'\n", in_key, getEntityDef()->m_entity_name)

    }

  return UNKNOW_COLUMN_ENTITY;
}


int Record::setValueForKeyAtIndex(const char* in_new_value, unsigned int in_key_index)
{
  unsigned int			tmp_value_length = 0;
  const unsigned int*	tmp_entity_def_sizes = getEntityDef()->m_entity_columns_sizes;

  if (in_key_index < getEntityDef()->m_entity_columns_count)
    {
      if (in_new_value != NULL)
        {
          tmp_value_length = strlen(in_new_value);
          if (tmp_entity_def_sizes == NULL || tmp_value_length <= tmp_entity_def_sizes[in_key_index])
            {
              if (_m_columns_record[in_key_index] != NULL)
                cl_free_str(&_m_columns_record[in_key_index]);

              _m_columns_record[in_key_index] = cl_alloc_str(tmp_value_length);
              strcpy(_m_columns_record[in_key_index], in_new_value);

              return NOERR;
            }
          else
            return OUT_OF_COLUMN_ENTITY_SIZE_LIMIT;
        }
      else
        {
          if (_m_columns_record[in_key_index] != NULL){
              cl_free_str(&_m_columns_record[in_key_index]);
            }
        }
      return NOERR;

    }

  return UNKNOW_COLUMN_ENTITY;
}

const char* Record::getValueForKey(const char* in_key) const
{
  const char**	tmp_entity_def_columns_names = getEntityDef()->m_entity_columns_names;

  if (is_empty_string(in_key) == false)
    {
      for (unsigned int tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
        {
          if (is_empty_string(tmp_entity_def_columns_names[tmp_index]) == false && strcmp(tmp_entity_def_columns_names[tmp_index], in_key) == 0)
            return _m_columns_record[tmp_index];
        }
    }

  LOG_ERROR(CLIENT_SESSION, "Cannot GET value for unknow column named '%s' in entity '%s'\n", in_key, getEntityDef()->m_entity_name);
  return NULL;
}


const char* Record::getOriginalValueForKey(const char* in_key) const
{
  const char**	tmp_entity_def_columns_names = getEntityDef()->m_entity_columns_names;

  if (_m_originals_columns_record != NULL)
    {
      if (is_empty_string(in_key) == false)
        {
          for (unsigned int tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
            {
              if (is_empty_string(tmp_entity_def_columns_names[tmp_index]) == false && strcmp(tmp_entity_def_columns_names[tmp_index], in_key) == 0)
                return _m_originals_columns_record[tmp_index];
            }
        }

      LOG_ERROR(CLIENT_SESSION, "Cannot GET original value for unknow column named '%s' in entity '%s'\n", in_key, getEntityDef()->m_entity_name);
    }

  return NULL;
}


const char* Record::getValueForKeyAtIndex(unsigned int in_key_index) const
{
  if (in_key_index < getEntityDef()->m_entity_columns_count)
    return _m_columns_record[in_key_index];

  return NULL;
}


int Record::getEntityDefSignatureId() const
{
  const entity_def*   tmp_def = getEntityDef();

  if (tmp_def != NULL)
    return tmp_def->m_entity_signature_id;

  return 0;
}


/**
 * Charger l'enregistrement de la base de données
 * @param in_primary_key_value
 * @return
 */
int Record::loadRecord(const char* in_primary_key_value)
{
  int		tmp_result = EMPTY_OBJECT;

  if (is_empty_string(in_primary_key_value) == false)
    {
      tmp_result = cl_load_record(CLIENT_SESSION, getEntityDef()->m_entity_signature_id, _m_columns_record, in_primary_key_value);
      if (tmp_result == NOERR)
        {
          cloneColumns();
        }
    }
  return tmp_result;
}


/**
 * Supprimer l'enregitrement de la base de donnees
 * @return
 */
int Record::deleteRecord()
{
  int		tmp_status =  NOERR;

  if (is_empty_string(getIdentifier()) == FALSE || getEntityDef()->m_primary_key != NULL)
    {
      tmp_status = cl_delete_record(CLIENT_SESSION, getEntityDef()->m_entity_signature_id, _m_columns_record);

//      if (tmp_status == NOERR)
//        {
//          unlockRecord();
//        }
    }

  return tmp_status;
}


/**
 * Supprimer plusieurs enregitrements de la base de donnees
 * @param in_signature_id
 * @param in_where_clause
 * @return
 */
int Record::deleteRecords(int in_signature_id, char* in_where_clause)
{
  return cl_delete_records(CLIENT_SESSION, in_signature_id, in_where_clause);
}

/**
 * Enregistrer les modifications dans la base de donnees
 * @param in_original_record
 * @return
 */
int Record::saveRecord(Record *in_original_record)
{
  int				tmp_status = NOERR;

  net_session		*tmp_app = CLIENT_SESSION;

  if (is_empty_string(getIdentifier()) || (getEntityDef()->m_primary_key != NULL && _m_originals_columns_record == NULL))
    {
      tmp_status = cl_insert_record(tmp_app, getEntityDef()->m_entity_signature_id, _m_columns_record);
    }
  else
    {
      if (in_original_record != NULL)
        tmp_status = cl_save_record(tmp_app, getEntityDef()->m_entity_signature_id, _m_columns_record, in_original_record->_m_columns_record);
      else
        tmp_status = cl_save_record(tmp_app, getEntityDef()->m_entity_signature_id, _m_columns_record, _m_originals_columns_record);

//      if (tmp_status == NOERR)
//        unlockRecord();
    }

  if (tmp_status == NOERR)
    cloneColumns();

  return tmp_status;
}


/**
 * Enregistrer les modifications dans la base de donnees
 * @return
 */
int Record::saveRecord()
{
  return saveRecord(NULL);
}


/**
 * Ajouter un enregistrement dans la base de données
 * @return
 */
int Record::insertRecord()
{
  int		tmp_status = cl_insert_record(CLIENT_SESSION, getEntityDef()->m_entity_signature_id, _m_columns_record);

  if (tmp_status == NOERR)
    {
      cloneColumns();
    }

  return tmp_status;
}


/**
 * Comparer deux enregistrements
 * @param in_record
 * @return
 */
int Record::compareTo(Record *in_record) const
{
  return compareTo(in_record, (char*)getEntityDef()->m_entity_columns_names[0]);
}


/**
 * Comparer les champs de deux enregistrements
 * @param in_record
 * @param in_column_name
 * @return
 */
int Record::compareTo(Record *in_record, char* in_column_name) const
{
  if (is_empty_string(in_column_name))
    return compareField(in_record, (char*)getEntityDef()->m_entity_columns_names[0]);
  else
    return compareField(in_record, in_column_name);
}


/**
 * Comparer les deux champs de deux enregistrements
 * @param in_record
 * @param in_field_name
 * @return
 */
int Record::compareField(Record *in_record, char* in_field_name) const
{
  const char*       tmp_first_value = getValueForKey(in_field_name);
  const char*	tmp_second_value = in_record->getValueForKey(in_field_name);

  return compare_values(tmp_first_value, tmp_second_value);
}


int Record::lockRecord(bool in_check_sync)
{
  int	    tmp_status = NOERR;

  if (_m_record_status != RECORD_STATUS_OWN_LOCK)
    {
      if (is_empty_string(getIdentifier()))
        {
          tmp_status = NOERR;
        }
      else
        {
          if (in_check_sync)
            tmp_status = cl_lock_record(CLIENT_SESSION, getEntityDef()->m_entity_signature_id, _m_columns_record[0], TRUE, _m_originals_columns_record);
          else
            tmp_status = cl_lock_record(CLIENT_SESSION, getEntityDef()->m_entity_signature_id, _m_columns_record[0], FALSE, NULL);
        }

      switch (tmp_status)
        {
        case NOERR:
          _m_record_status = RECORD_STATUS_OWN_LOCK;
          break;

        case DB_RECORD_LOCK_UNAVAILABLE:
          _m_record_status = RECORD_STATUS_LOCKED;
          break;

        case DB_RECORD_OUT_OF_SYNC:
        default:
          _m_record_status = RECORD_STATUS_OUT_OF_SYNC;
          break;
        }
    }

  return tmp_status;
}

int Record::lockRecordStatus() const
{
  return _m_record_status;
}


int Record::setRecordStatus(int in_record_status)
{
  _m_record_status = in_record_status;

  return NOERR;
}


int Record::unlockRecord()
{
  int		tmp_status = NOERR;

  if (_m_record_status != RECORD_STATUS_OWN_LOCK || is_empty_string(getIdentifier()))
    {
      _m_record_status = RECORD_STATUS_MODIFIABLE;
      tmp_status = NOERR;
    }
  else
    {
      tmp_status = cl_unlock_record(CLIENT_SESSION, getEntityDefSignatureId(), _m_columns_record[0]);
      if(tmp_status == NOERR)
        _m_record_status = RECORD_STATUS_MODIFIABLE;
    }

  if (tmp_status != NOERR)
    {
      LOG_ERROR(CLIENT_SESSION, "Unlock error %i on entity %s (%s)\n", tmp_status, getEntityDef()->m_entity_name, getIdentifier());
    }

  return tmp_status;
}


QString Record::serialize() const
{
  QString                     tmp_result = QString("%1").arg(getEntityDef()->m_entity_signature_id) + SEPARATOR_CHAR;
  unsigned int 		tmp_index = 0;

  if (_m_columns_record != NULL)
    {
      for (tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
        {
          if (_m_columns_record[tmp_index] != NULL)
            tmp_result += _m_columns_record[tmp_index];

          tmp_result += SEPARATOR_CHAR;
        }
    }

  return tmp_result;

}


void Record::deserialize(const QString & in_serialized_record)
{
  const char		*tmp_current_data_ptr = NULL;
  std::string         tmp_str = in_serialized_record.toStdString();

  const char          *tmp_data_ptr = tmp_str.c_str();
  char		tmp_column_value[MAX_COLUMN_LENGTH];

  // Verifier le type de l'objet
  tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_column_value, SEPARATOR_CHAR);
  if (is_empty_string(tmp_column_value) == 1)
    return;

  if (atoi(tmp_column_value) != getEntityDef()->m_entity_signature_id)
    return;

  for (unsigned int tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
    {
      tmp_current_data_ptr = net_get_field(0, tmp_data_ptr, tmp_column_value, SEPARATOR_CHAR);
      setValueForKeyAtIndex(tmp_column_value, tmp_index);

      tmp_data_ptr = tmp_current_data_ptr;
    }

  cloneColumns();
}


bool Record::hasChangedValues() const
{
  if (_m_originals_columns_record != NULL)
    {
      for (unsigned int tmp_index = 0; tmp_index < getEntityDef()->m_entity_columns_count; tmp_index++)
        {
          if (compare_values(_m_originals_columns_record[tmp_index], _m_columns_record[tmp_index]) != 0)
            return true;
        }
    }
  else
    return true;

  return false;
}

