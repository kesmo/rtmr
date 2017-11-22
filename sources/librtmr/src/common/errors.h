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

#ifndef ERRORS_H_
#define ERRORS_H_

#include <stddef.h>
#include "constants.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _error
{
	int		id;
	const char	*description;
}
error;

static const error errors_list[]  = {

        {MEM_ALLOC_ERROR, "Mémoire insuffusante."},
        {NO_LICENSE_AVAILABLE, "Aucune licence n'est disponible."},
        {LICENSE_CONNEXIONS_COUNT_ERROR, "Le nombre maximum de licenses utilisateurs est dépassé.\nLa connexion est impossible."},
        {LICENSE_TIME_ERROR, "La license a expirée.\nLa connexion est impossible."},
        {NET_INIT_CONNEXION, "L'initialisation de la connexion a échouée."},
        {NET_CONNEXION_ERROR, "La connexion a échouée."},
        {NET_SOCKET_ERROR, "La création du point de connexion (socket) a échouée."},
        {NET_GET_HOST_NAME_ERROR, "La récupération du nom d'hôte a échouée."},
        {NET_GET_HOST_BY_NAME_ERROR, "La récupération de l'hôte à partir du nom a échouée."},
        {NET_RCV_PAQUET_ERROR, "La réception de données a échouée."},
        {NET_SEND_PAQUET_ERROR, "L'envoie de données a échoué."},
        {NET_RCV_ACK_ERROR, "La réception de l'aquittement a échouée."},
        {NET_SEND_ACK_ERROR, "L'envoie de l'aquittement a échoué."},
        {NET_SEND_TIMEOUT, "L'envoie de données a échoué.\nL'hôte distant ne répond pas."},
        {NET_OUT_OF_REQUEST_LIMIT_SIZE_ERROR, "La requête est trop longue."},
        {NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR, "La réponse est trop longue."},
        {NET_REQUEST_UNKNOW, "La requête est inconnue."},
        {NET_PROTOCOL_VERSION_INCOMPATIBLE, "Les protocoles d'échanges de données client/serveur ne sont pas compatibles."},
        {NET_RECV_TIMEOUT, "La réception de données a échoué.\nL'hôte distant ne répond pas."},
        {NET_LIB_PROTOCOL_VERSION_INCOMPATIBLE, "Les protocoles d'échanges de données client/bibliothèque ne sont pas compatibles."},
        {NET_CONNEXION_CLOSED, "La connexion a été fermée."},

        {DB_CONNEXION_ERROR, "La connexion à la base de données a échouée."},
        {DB_UNKNOW_USER_ERROR, "L'utilisateur est inconnu."},
        {DB_INCORRECT_PASSWORD_ERROR, "Le mot de passe est incorrect."},

        {DB_USER_ACCOUNT_CREATION_ERROR, "La création du compte a échoué."},
        {DB_USER_ACCOUNT_ACTIVATION_ERROR, "L'activation du compte n'a pu être réalisée."},
        {DB_USER_ACCOUNT_ALREADY_ACTIVATED, "Le compte a déjà  été activé."},

        {DB_SQL_FUNCTION_ERROR, "Une erreur est survenue lors de l'appel à une fonction de la base de données."},
        {DB_SQL_NO_ROW_FOUND, "Aucune donnée n'a été trouvée."},
        {DB_SQL_TOO_MUCH_ROWS_FOUND, "Plusieurs données ont été trouvées."},
        {DB_SQL_ILLEGAL_STATEMENT, "L'instruction sql est incorrecte."},
        {DB_SQL_ERROR, "Une erreur sql est survenue."},

        {DB_USER_ALREADY_EXISTS, "L'identifiant saisi est déjà utilisé."},

        {DB_FILE_ACCESS_DENIED, "L'accès au document n'est pas autorisé."},

        {DB_GROUP_DOESNT_OWN_GROUP, "Le groupe n'est pas propriétaire du sous-groupe."},
        {DB_GROUP_DOESNT_OWN_USER, "Le groupe n'est pas propriétaire de l'utilisateur."},

        {UNKNOW_ENTITY, "L'entité est inconnue."},
        {UNKNOW_LDAP_TYPE_ENTRY, "L'entité LDAP est inconnue."},

        {EMPTY_OBJECT, "Un paramètre passé est nul ou vide."},
        {UNKNOW_COLUMN_ENTITY, "La colonne de l'entité est inconnue."},
        {OUT_OF_COLUMN_ENTITY_SIZE_LIMIT, "La valeur est trop grande pour la colonne."},
        {ENTITY_COLUMNS_COUNT_ERROR, "Le nombre de colonnes de l'entité est incorrect."},
 
        {SYSTEM_USER_HOME_CREATION_ERROR, "Une erreur est survenue lors de la création de l'espace disque."},
        {SYSTEM_USERS_DISK_ERROR, "Le système d'espace disque est incorrect."},
        {SYSTEM_SEND_MAIL_ERROR, "L'envoie du courriel a échoué."},

        {FILE_NOT_EXISTS, "Le document n'existe pas."},
        {FILE_ALREADY_EXISTS, "Le document existe déjà."},
        {FILE_CREATION_ERROR, "Impossible de créer le document."},
        {FILE_RENAMING_ERROR, "Impossible de renommer le document."},
        {FILE_OPENING_ERROR, "Impossible de créer/ouvrir le document."},
        {FILE_READING_ERROR, "Impossible de lire dans le document."},
        {FILE_WRITING_ERROR, "Impossible d'écrire dans le document."},
        {FILE_EMPTY_ERROR, "Le fichier est vide."},

        {GROUP_NAME_EMPTY, "Le nom du groupe ne peut être vide."},
        {GROUP_PARENT_EMPTY, "Le groupe parent doit être sélectionné."},
        {GROUP_CANT_BE_PARENT_OF_HIMSELF, "Le groupe ne peut être son propre parent"},

        {CONTACT_NAME_EMPTY, "Le nom, prénom ou société du contact doit être saisi."},

        {LDAP_CONNEXION_ERROR, "La connexion au serveur LDAP n'a pu être établie."},
        {LDAP_INVALID_CREDENTIAL, "Le nom d'utilisateur ou le mot de passe LDAP est incorrect."},
        {LDAP_QUERY_ERROR, "Une erreur lors de l'envoie d'une requête LDAP est survenue."},
        {LDAP_NO_ENTRY_FOUND, "Aucune donnée LDAP n'a été trouvée"},
        {LDAP_TOO_MUCH_ENTRIES_FOUND, "Plusieurs données LDAP ont été trouvées."},

        {LDAP_DB_USER_SYNC_ERROR, "Une erreur de synchronisation LDAP est survenue."},

        {UNKNOW_ERROR, "Une erreur inconnue est survenue."}
};

/***********************************************************************************	*
*	get_error_message																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Renvoie la description du message d'erreur																		. 							*
*************************************************************************************/
const char	*get_error_message(int);

#ifdef __cplusplus
}
#endif

#endif /*ERRORS_H_*/
