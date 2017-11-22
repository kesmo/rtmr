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

#include "utilities.h"
#include "constants.h"
#include <ctype.h>
#include <string.h>

#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <time.h>

const char	*crypting_key = "0dc43e4ae9f86c17c4744ac5d94ebe8fcb8b6341916915155be5db4d2e991e203a5510a0c4c9e4be40038b9ae1c88ffda6b04bf5ba6c74d1fc9f307d2d1f97fd682b5e46fdbaed95c526bb245cf1f25a5c39a044b7a732deebcc24153be35415843d41036c4555bc6061be4c46f94f3aa5a2bc374ecd75ab46cb2ee22ecafe28";

/*
 * Renvoie TRUE si la chaine de caractères est vide
 */
DLLEXPORT int DLLCALL is_empty_string(const char* in_string)
{
    return (in_string == NULL || *in_string == '\0');
}


/**
 * Comparer deux chaînes de caractères
 * @param in_first_value
 * @param in_second_value
 * @return
 */
DLLEXPORT int DLLCALL compare_values(const char* in_first_value, const char* in_second_value)
{
    int tmp_result = 0;

    if (is_empty_string(in_first_value) == FALSE)
    {
        if (is_empty_string(in_second_value) == FALSE)
        {
            tmp_result = strcmp(in_first_value, in_second_value);
        }
        else
        {
            tmp_result = 1;
        }
    }
    else if (is_empty_string(in_second_value) == FALSE)
    {
        tmp_result = -1;
    }

    return tmp_result;
}

char *get_app_param(const char *in_file_name, const char *in_param_name)
{
	char		*tmp_param_value = NULL;

	FILE		*tmp_file = fopen(in_file_name, "r");

	if (tmp_file != NULL)
	{
		tmp_param_value = get_app_param_from_file(tmp_file, in_param_name);

		fclose(tmp_file);
    }
	else
	{
		fprintf(stderr, "get_app_param : unable to open %s\n", in_file_name);
		fflush(stderr);
	}

	return tmp_param_value;
}

char *get_app_param_from_file(FILE *in_file, const char *in_param_name)
{
	char		tmp_line[MAX_APP_PARAM_LINE_SIZE];
	char		*tmp_ptr = NULL;
	char		*tmp_param_value = NULL;

	if (in_file != NULL)
	{
		rewind(in_file);
		while (fgets(tmp_line, MAX_APP_PARAM_LINE_SIZE, in_file) != NULL)
		{
				if (starts_with(trim(tmp_line), in_param_name) == 1)
				{
					tmp_ptr = tmp_line;
					if ((tmp_ptr + strlen(in_param_name))[0] == '=')
					{
						tmp_ptr += strlen(in_param_name);
						trim(++tmp_ptr);
						tmp_param_value = (char*)malloc(strlen(tmp_ptr) + 1);
						strcpy(tmp_param_value, tmp_ptr);
					}
				}
		}
    }

	return tmp_param_value;
}


char* trim (char *in_str)
{
      char *tmp_read_buf, *tmp_buf;

      if (in_str)
      {
            for (tmp_read_buf = tmp_buf = in_str; *tmp_read_buf; )
            {
                  while (*tmp_read_buf && (isspace (*tmp_read_buf)))
                	  tmp_read_buf++;
                  if (*tmp_read_buf && (tmp_buf != in_str))
                        *(tmp_buf++) = ' ';
                  while (*tmp_read_buf && (!isspace (*tmp_read_buf)))
                        *(tmp_buf++) = *(tmp_read_buf++);
            }
            *tmp_buf = '\0';
      }

      return (in_str);
}


int starts_with(const char *in_src_str, const char *in_search_str)
{
	char	*tmp_src_ptr = (char*)in_src_str;
	char	*tmp_search_ptr = (char*)in_search_str;

	int		tmp_result = 0;

	while (*tmp_src_ptr && *tmp_search_ptr && (tmp_result = *(tmp_src_ptr++) == *(tmp_search_ptr++)) == 1);

	return tmp_result;
}


char* format_string(const char *in_src_str, char *in_out_dest_str)
{
	const char	*tmp_src_ptr = in_src_str;
	char		*tmp_out_result_ptr = in_out_dest_str;

	if (in_src_str)
	{
		while (*tmp_src_ptr)
		{
			if (*tmp_src_ptr == '\'' || *tmp_src_ptr == '\"' || *tmp_src_ptr == '\\')
				*(tmp_out_result_ptr++) = '\\';

			*tmp_out_result_ptr = *tmp_src_ptr;
			tmp_src_ptr++;
			tmp_out_result_ptr++;
		}
		*tmp_out_result_ptr = '\0';
	}

	return tmp_out_result_ptr;
}


char *print_formated_column(char *in_out_str, const char *in_column_value_str, const char *in_column_format)
{
	char	*tmp_str = in_out_str;

	if (in_column_value_str == NULL)	return in_out_str;
	if (in_out_str == NULL)	return NULL;

	if (in_column_format)
	{
		if (compare_values(in_column_format, DATE_FORMAT) == 0 && compare_values(in_column_value_str, CLIENT_MACRO_NOW) == 0)
		{
			tmp_str += sprintf(in_out_str, "%s", NOW);
		}
		else
		{
			tmp_str += sprintf(in_out_str, in_column_format, in_column_value_str);
		}
	}
	else
	{
		tmp_str += sprintf(in_out_str, STANDARD_FORMAT, in_column_value_str);
	}

	return tmp_str;
}


char *nprint_formated_column(char *in_out_str, int in_size, const char *in_column_value_str, const char *in_column_format)
{
	char		*tmp_str = in_out_str;
	const char	*tmp_src_str = in_column_value_str;
	const char	*tmp_format_ptr = NULL;

	int		tmp_index = 0;

	if (in_column_value_str == NULL)	return in_out_str;
	if (in_out_str == NULL)	return NULL;

	if (in_column_format)
	{
		if (compare_values(in_column_format, DATE_FORMAT) == 0)
		{
			if (strncmp(in_column_value_str, CLIENT_MACRO_NOW, in_size) == 0)
			{
				tmp_str += sprintf(in_out_str, "%s", NOW);
				return tmp_str;
			}
		}

		tmp_format_ptr = in_column_format;
	}
	else
	{
		tmp_format_ptr = STANDARD_FORMAT;
	}

	while (tmp_format_ptr && tmp_src_str[0] != '\0' && tmp_format_ptr[0] != '%')
	{
		tmp_str[0] = tmp_format_ptr[0];
		tmp_str++;
		tmp_format_ptr++;
	}
	if (tmp_format_ptr[0] == '%')
		tmp_format_ptr+=2;

	while (tmp_src_str && tmp_src_str[0] != '\0' && tmp_index < in_size)
	{
		tmp_str[0] = tmp_src_str[0];
		tmp_str++;
		tmp_src_str++;
		tmp_index++;
	}

	while (tmp_format_ptr && tmp_format_ptr[0] != '\0')
	{
		tmp_str[0] = tmp_format_ptr[0];
		tmp_str++;
		tmp_format_ptr++;
	}
	tmp_str[0] = '\0';

	return tmp_str;
}



void free_rows_and_columns(char ****in_rows_columns, int in_rows_count, int in_columns_count)
{
	int tmp_row_index = 0;
	int tmp_column_index = 0;

	if (in_rows_columns != NULL && (*in_rows_columns) != NULL)
	{
		for (tmp_row_index = 0; tmp_row_index < in_rows_count; tmp_row_index++)
		{
			if (((*in_rows_columns)[tmp_row_index]) != NULL)
			{
				for (tmp_column_index = 0; tmp_column_index < in_columns_count; tmp_column_index++)
				{
					if (((*in_rows_columns)[tmp_row_index][tmp_column_index]) != NULL)
						free((*in_rows_columns)[tmp_row_index][tmp_column_index]);
				}
				free((*in_rows_columns)[tmp_row_index]);
			}
		}
		free(*in_rows_columns);
		*in_rows_columns = NULL;
	}
}


DLLEXPORT char* DLLCALL decrypt_str(const char *in_crypted_str)
{
	unsigned int					tmp_start_index = 0;
	unsigned int					tmp_index = 0;
	unsigned int					tmp_key_index = 0;
	unsigned int					tmp_length = 0;
	char							tmp_length_str[3];
	char							tmp_current_key_char;
	char							*tmp_decrypted_str = NULL;

	if (is_empty_string(in_crypted_str))	return NULL;

	tmp_start_index = hexa_str_to_int(in_crypted_str, 2);
	tmp_key_index = tmp_start_index;

	tmp_current_key_char = crypting_key[tmp_key_index%256];
	tmp_length_str[0] = ((in_crypted_str[2] - 'a') << 4) ^ tmp_current_key_char;
	tmp_length_str[0] = (tmp_length_str[0] & 0XF0) | ((((in_crypted_str[3] - 'a') & 0x0F) ^ tmp_current_key_char) & 0x0F);
	tmp_key_index++;
	tmp_current_key_char = crypting_key[tmp_key_index%256];
	tmp_length_str[1] = ((in_crypted_str[4] - 'a') << 4) ^ tmp_current_key_char;
	tmp_length_str[1] = (tmp_length_str[1] & 0XF0) | ((((in_crypted_str[5] - 'a') & 0x0F) ^ tmp_current_key_char) & 0x0F);
	tmp_length_str[2] = '\0';
	tmp_key_index++;

	tmp_length = hexa_str_to_int(tmp_length_str, 2);
	tmp_decrypted_str = (char*)malloc(tmp_length + 1);

	for (tmp_index = 0; tmp_index < tmp_length; tmp_index++)
	{
		tmp_current_key_char = crypting_key[tmp_key_index%256];
		tmp_decrypted_str[tmp_index] = ((in_crypted_str[tmp_index * 2 + 6] - 'a') << 4) ^ tmp_current_key_char;
		tmp_decrypted_str[tmp_index] = (tmp_decrypted_str[tmp_index] & 0XF0) | ((((in_crypted_str[tmp_index * 2 + 7] - 'a') & 0x0F) ^ tmp_current_key_char) & 0x0F);
		tmp_key_index++;
	}
	tmp_decrypted_str[tmp_length] = '\0';

	return tmp_decrypted_str;
}

DLLEXPORT char* DLLCALL encrypt_str(const char *in_decrypted_str)
{
	unsigned int					tmp_start_index = 0;
	unsigned int					tmp_index = 0;
	unsigned int					tmp_key_index = 0;
	unsigned int					tmp_src_length = 0;
	unsigned int					tmp_length = 0;
	char									tmp_length_str[3];
	char									tmp_current_key_char;
	char									tmp_current_char;
	char									*tmp_crypted_str = NULL;

	if (is_empty_string(in_decrypted_str))	return NULL;

	tmp_start_index = (unsigned int)(256.0 * (rand() / (RAND_MAX + 1.0)));
	tmp_key_index = tmp_start_index;
	tmp_src_length = strlen(in_decrypted_str);
	tmp_length = MAX(tmp_src_length * 2 + 7, 64);;
	tmp_crypted_str = (char*)malloc(tmp_length);

	sprintf(tmp_crypted_str, "%.2x", tmp_start_index);

	sprintf(tmp_length_str, "%.2x", tmp_src_length);

	tmp_current_key_char = crypting_key[tmp_key_index%256];
	tmp_crypted_str[2] = ((tmp_length_str[0] ^ tmp_current_key_char) >> 4) + 'a';
	tmp_crypted_str[3] = ((tmp_length_str[0] ^ tmp_current_key_char) & 0x0F) + 'a';
	tmp_key_index++;
	tmp_current_key_char = crypting_key[tmp_key_index%256];
	tmp_crypted_str[4] = ((tmp_length_str[1] ^ tmp_current_key_char) >> 4) + 'a';
	tmp_crypted_str[5] = ((tmp_length_str[1] ^ tmp_current_key_char) & 0x0F) + 'a';
	tmp_key_index++;

	for (tmp_index = 0; tmp_index < tmp_src_length; tmp_index++)
	{
		tmp_current_key_char = crypting_key[tmp_key_index%256];
		tmp_crypted_str[tmp_index * 2 + 6] = ((in_decrypted_str[tmp_index] ^ tmp_current_key_char) >> 4) + 'a';
		tmp_crypted_str[tmp_index * 2 + 7] = ((in_decrypted_str[tmp_index] ^ tmp_current_key_char) & 0x0F) + 'a';
		tmp_key_index++;
	}

	for (tmp_index = tmp_src_length; tmp_index < (tmp_length - 7) / 2; tmp_index++)
	{
		tmp_current_char = (char)(64.0 * (rand() / (RAND_MAX + 1.0))) + 33;
		tmp_current_key_char = crypting_key[tmp_key_index%256];
		tmp_crypted_str[tmp_index * 2 + 6] = ((tmp_current_char ^ tmp_current_key_char) >> 4) + 'a';
		tmp_crypted_str[tmp_index * 2 + 7] = ((tmp_current_char ^ tmp_current_key_char) & 0x0F) + 'a';
		tmp_key_index++;
	}

	tmp_crypted_str[tmp_index * 2 + 6] = '\0';

	return tmp_crypted_str;
}

int hexa_str_to_int(const char* in_str, int count)
{
	int			tmp_return_value = 0;
	short	tmp_digit;
	short	i;

	for (i = count; i>0; i--)
	{
		if ((in_str[i-1] >= 'a') && (in_str[i-1] <= 'f'))
			tmp_digit = in_str[i-1]-'a' + 0xA;
		else if ((in_str[i-1] >= 'A') && (in_str[i-1] <= 'F'))
			tmp_digit = in_str[i-1]-'A' + 0xA;
		else
			tmp_digit = in_str[i-1]-'0';

		tmp_return_value += (int)pow(0x10,(count-i)) * tmp_digit;
	}
	return tmp_return_value;
}


int index_of_string_in_array(const char *in_search_string, const char **in_array)
{
	const char		**tmp_str = in_array;
	int						tmp_index = 0;

	while (tmp_str && (*tmp_str))
	{
		if (strcmp(*tmp_str, in_search_string) == 0)
			return tmp_index;

		tmp_str++;
		tmp_index++;
	}

	return -1;
}

/*************************************************************************************
*	net_get_field
*			recupere un champ dans une chaine selon le separateur
*		Parametres :
*			field_index : index du champ (a partir de 0)
*			data_str : chaine source
*			field : destination
*			field_sep : separateur
*		Retour :
*			derniere position de lecture
**************************************************************************************/
DLLEXPORT const char* DLLCALL net_get_field(int field_index, const char *data_str, char* field, char field_sep)
{
	char	*aStr = NULL;
	const char	*cStr = data_str;
	int		index = 0;

	if (data_str && field)
	{
		field[0] = '\0';
		aStr = strchr(cStr, field_sep);
		while (aStr != NULL && index < field_index)
		{
			index++;
			cStr = aStr + 1;
			aStr = strchr(cStr, field_sep);
		}

		if (index == field_index)
		{
			if (aStr)
			{
				strncpy(field, cStr, aStr - cStr);
				field[aStr - cStr] = '\0';
				cStr = aStr + 1;
			}
		}
	}
	return cStr;
}


int net_find_field(int field_index, const char **data_str, char field_sep)
{
	char	*aStr = NULL;
	int		index = 0;
	int		tmp_length = 0;

	if (data_str && (*data_str))
	{
		aStr = strchr((*data_str), field_sep);
		while (aStr != NULL && index < field_index)
		{
			index++;
			(*data_str) = aStr + 1;
			aStr = strchr((*data_str), field_sep);
		}

		if (index == field_index)
		{
			if (aStr)
			{
				tmp_length = aStr - (*data_str);
			}
		}
	}

	return tmp_length;
}



/*************************************************************************************
*	remove_char
*			Supprime toutes occurences d'un caractere dans une chaine
*		Parametres :
*			in_buffer : chaine source
*			a_character : caractere
*		Retour :
*			Aucun
**************************************************************************************/
int remove_char(char * in_buffer, char a_character)
{
	char *read_auxbuf = NULL;
	char *write_auxbuf = NULL;
	int		returnValue = 0;

	/* Verifications */
	if (in_buffer == NULL) return returnValue;

	read_auxbuf = in_buffer;
	write_auxbuf = in_buffer;
	while (*read_auxbuf) {
		if (*read_auxbuf != a_character) {
			*write_auxbuf++ = *read_auxbuf;
			returnValue++;
		}
		read_auxbuf++;
	}

	/* Terminer la chaine !! */
	*write_auxbuf = '\0';

	return returnValue;
}


DLLEXPORT int DLLCALL replace_char(char * in_buffer, char in_old_character, char in_new_character)
{
	char *write_auxbuf = NULL;
	int		returnValue = 0;

	if (in_buffer == NULL) return returnValue;

	write_auxbuf = in_buffer;
	while (*write_auxbuf) {
		if (*write_auxbuf == in_old_character) {
			*write_auxbuf = in_new_character;
			returnValue++;
		}
		write_auxbuf++;
	}

	return returnValue;

}
