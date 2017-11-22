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

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <stdio.h>
#include <time.h>


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef RTMR_LIBRARY_EXPORTS
    #if (defined(_WINDOWS) || defined(WIN32))
            #define DLLEXPORT   __declspec( dllexport )
            #define DLLCALL		__stdcall
    #else
            #define DLLEXPORT   __attribute__((visibility("default")))
            #define DLLCALL
    #endif
#else
    #ifdef RTMR_LIBRARY_IMPORTS
		#if (defined(_WINDOWS) || defined(WIN32))
                #define DLLEXPORT   __declspec( dllimport )
                #define DLLCALL		__stdcall
        #else
                #define DLLEXPORT
                #define DLLCALL
        #endif
    #else
        #define DLLEXPORT
        #define DLLCALL
    #endif
#endif


#define					LOG_LEVEL_NONE											0
#define					LOG_LEVEL_ERRORS										1
#define					LOG_LEVEL_TRACES										2

#if ((defined(_WINDOWS) || defined(WIN32))) && !(defined(__GNUC__))
	#define LOG(session, level, format, ...) \
		if (session) \
		{ \
			if (session->m_log_trace_file && session->m_log_level >= level) \
			{ \
				time_t	tmp_ctime = time(NULL); \
				struct 	tm	*tmp_mtime = localtime ( &tmp_ctime ); \
				fprintf (session->m_log_trace_file, "%.2i:%.2i:%.2i [INFO]> " , tmp_mtime->tm_hour, tmp_mtime->tm_min, tmp_mtime->tm_sec); \
				fprintf (session->m_log_trace_file, format , __VA_ARGS__); \
				fflush(session->m_log_trace_file); \
				fflush (stderr); \
			} \
			else if (session->m_debug)\
			{ \
				fprintf (stdout, format , __VA_ARGS__); \
				fflush (stdout); \
			} \
		} \
		else \
		{ \
			fprintf (stdout, format , __VA_ARGS__); \
			fflush (stdout); \
		} \

		#define LOG_TRACE(session, format, ...) \
			LOG(session, LOG_LEVEL_TRACES, format, __VA_ARGS__) \


	#define LOG_ERROR(session, format, ...) \
		if (session) \
		{ \
			if (session->m_log_trace_file && session->m_log_level >= LOG_LEVEL_ERRORS) \
			{ \
				time_t	tmp_ctime = time(NULL); \
				struct 	tm	*tmp_mtime = localtime ( &tmp_ctime ); \
				fprintf (session->m_log_trace_file, "%.2i:%.2i:%.2i [ERROR]> " , tmp_mtime->tm_hour, tmp_mtime->tm_min, tmp_mtime->tm_sec); \
				fprintf (session->m_log_trace_file, format , __VA_ARGS__); \
				fflush(session->m_log_trace_file); \
			} \
			else if (session->m_debug)\
			{ \
				fprintf (stderr, format , __VA_ARGS__); \
				fflush (stderr); \
			} \
		} \
		else \
		{ \
			fprintf (stderr, format , __VA_ARGS__); \
			fflush (stderr); \
		} \

	#define LOG_SOCK_ERRORNO(session, function) \
        LOG_ERROR(session, "%s : socket error : %i\n", function, WSAGetLastError())

#else
        #define LOG(session, level, format, args...) \
			if (session) \
			{ \
				if (session->m_log_trace_file && session->m_log_level >= level) \
				{ \
					time_t		tmp_ctime = time(NULL); \
					struct 	tm	*tmp_mtime = localtime ( &tmp_ctime ); \
					fprintf (session->m_log_trace_file, "%.2i:%.2i:%.2i [INFO]> " , tmp_mtime->tm_hour, tmp_mtime->tm_min, tmp_mtime->tm_sec); \
					fprintf (session->m_log_trace_file, format , ##args); \
					fflush(session->m_log_trace_file); \
				} \
				else if (session->m_debug)\
				{ \
					fprintf (stdout, format , ##args); \
					fflush (stdout); \
				} \
			} \
			else \
			{ \
				fprintf (stdout, format , ##args); \
				fflush (stdout); \
			} \

		#define LOG_TRACE(session, format, args...) \
			LOG(session, LOG_LEVEL_TRACES, format, ##args) \

        #define LOG_ERROR(session, format, args...) \
			if (session) \
			{ \
				if (session->m_log_trace_file && session->m_log_level >= LOG_LEVEL_ERRORS) \
				{ \
					time_t	tmp_ctime = time(NULL); \
					struct 	tm	*tmp_mtime = localtime ( &tmp_ctime ); \
					fprintf (session->m_log_trace_file, "%.2i:%.2i:%.2i [ERROR]> " , tmp_mtime->tm_hour, tmp_mtime->tm_min, tmp_mtime->tm_sec); \
					fprintf (session->m_log_trace_file, format , ##args); \
					fflush(session->m_log_trace_file); \
				} \
				else if (session->m_debug)\
				{ \
					fprintf (stderr, format , ##args); \
					fflush (stderr); \
				} \
			} \
			else \
			{ \
				fprintf (stderr, format , ##args); \
				fflush (stderr); \
			} \

			#define LOG_SOCK_ERRORNO(session, function) \
					int	tmp_error = errno; \
					LOG_ERROR(session, "%s : socket error : %s (%i)\n", function, strerror(tmp_error), tmp_error)

#endif


#define LOG_ERRORNO(session, function) \
	int	tmp_error = errno; \
        LOG_ERROR(session, "%s : %s (%i)\n", function, strerror(tmp_error), tmp_error)

#define MIN(A,B) (A < B ? A : B)
#define MAX(A,B) (A > B ? A : B)

typedef void (net_callback_fct) (unsigned long long int in_index, unsigned long long int in_count, unsigned long long int in_step, const char* in_message);

DLLEXPORT int DLLCALL is_empty_string(const char*);
DLLEXPORT int DLLCALL compare_values(const char* in_first_value, const char* in_second_value);
char *get_app_param(const char *in_file_name, const char *in_param_name);
char *get_app_param_from_file(FILE *in_file, const char *in_param_name);
char* trim (char *in_str);
int starts_with(const char *in_src_str, const char *in_search_str);
char* format_string(const char *in_src_str, char *in_out_dest_str);
char *print_formated_column(char *in_out_str, const char *in_column_value_str, const char *in_column_format);
char *nprint_formated_column(char *in_out_str, int in_size, const char *in_column_value_str, const char *in_column_format);
void free_rows_and_columns(char ****in_rows_columns, int in_rows_count, int in_columns_count);

DLLEXPORT char* DLLCALL decrypt_str(const char *in_crypted_str);
DLLEXPORT char* DLLCALL encrypt_str(const char *in_decrypted_str);

int hexa_str_to_int(const char* in_str, int count);

int index_of_string_in_array(const char *in_search_string, const char **in_array);

DLLEXPORT const char* DLLCALL net_get_field(int field_pos, const char *data_str, char* field, char field_sep);
int net_find_field(int field_index, const char **data_str, char field_sep);
int remove_char(char * in_buffer, char a_character);
DLLEXPORT int DLLCALL replace_char(char * in_buffer, char in_old_character, char in_new_character);

#ifdef __cplusplus
}
#endif

#endif /*UTILITIES_H_*/
