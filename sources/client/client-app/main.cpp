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

#include "session.h"
#include "gui/mainwindow.h"

#include <QTextCodec>
#include <QIODevice>

#include "clientmodule.h"
#include "netcommon.h"

static QMap<ClientModule*, QLibrary*>	EXTERNALS_MODULES_LIST;

static void loadExternalModules()
{
#ifndef EXTERNALS_MODULES_DIR
    QDir            tmp_modules_dir = QDir(QApplication::applicationDirPath()+"/modules");
#else
    QDir            tmp_modules_dir = QDir(EXTERNALS_MODULES_DIR);
#endif
    QStringList     tmp_filternames;
    QFileInfoList   tmp_files_infos_list;

    ClientModule                *tmp_module_instance = NULL;

    create_module_instance_fct  tmp_create_module_instance_fct = NULL;

    if (tmp_modules_dir.exists())
    {
#ifdef __APPLE__
         tmp_filternames << "*.dylib";
#else
         tmp_filternames << "*.cmo";
#endif

	 tmp_files_infos_list = tmp_modules_dir.entryInfoList(tmp_filternames, QDir::Files);

         fprintf(stdout, "Reading modules folder named %s\n", tmp_modules_dir.absolutePath().toStdString().c_str());

	 if (!tmp_files_infos_list.isEmpty())
	 {
	     foreach(QFileInfo tmp_file_info, tmp_files_infos_list)
	     {

			 if (tmp_file_info.isFile())
			 {
				QLibrary   *tmp_library = new QLibrary(tmp_file_info.filePath());

				fprintf(stdout, "Loading library <%s>.\n", tmp_file_info.filePath().toStdString().c_str());
				if (!tmp_library->load())
				{
					fprintf(stderr, "%s\n", tmp_library->errorString().toStdString().c_str());
					delete tmp_library;
					continue;
				}

				fprintf(stdout, "Retreiving function <%s>.\n", MODULE_INIT_INSTANCE_ENTRY_NAME);
				tmp_create_module_instance_fct = (create_module_instance_fct) tmp_library->resolve(MODULE_INIT_INSTANCE_ENTRY_NAME);

				if (tmp_create_module_instance_fct == NULL)
				{
					fprintf(stderr, "%s\n", tmp_library->errorString().toStdString().c_str());
					tmp_library->unload();
					delete tmp_library;
					continue;
				}

				fprintf(stdout, "Module initialization.\n");
				tmp_module_instance = (*tmp_create_module_instance_fct)();
				if (tmp_module_instance == NULL)
				{
					fprintf(stderr, "%s\n", tmp_library->errorString().toStdString().c_str());
					tmp_library->unload();
					delete tmp_library;
				}
				else
				{
                    fprintf(stdout, "Adding module <%s> to application.\n", tmp_module_instance->getModuleName().toStdString().c_str());
                    if (!Session::instance()->addExternalModule(tmp_module_instance))
					{
						fprintf(stderr, "Same module has already been added.\n");
						tmp_library->unload();
						delete tmp_library;
					}else
					    EXTERNALS_MODULES_LIST[tmp_module_instance] = tmp_library;
				}
			}
	     }
	 }
    }
    else
    {
        fprintf(stderr, "There is no modules folder named %s.\n", tmp_modules_dir.absolutePath().toStdString().c_str());
    }

    fflush(stdout);
    fflush(stderr);
}


/**
  Programme principal
**/
int main(int argc, char *argv[])
{
    int tmp_app_return = 0;
    destroy_module_instance_fct  tmp_destroy_module_instance_fct = NULL;

    for(int tmp_index = 0; tmp_index < argc; tmp_index++)
    {
	if (strcmp(argv[tmp_index], "-version") == 0)
	{
	    fprintf(stdout, "%s%c", APP_VERSION, 0xA);
	    return NOERR;
	}
    }

    QApplication     tmp_app(argc, argv);
    MainWindow      tmp_main_wnd;

    // Chargements des plugins
    loadExternalModules();

    // Affichage de la fenêtre principale
    tmp_main_wnd.show();
    tmp_main_wnd.logon();

    tmp_app_return = tmp_app.exec();

    foreach(ClientModule* clientModule, EXTERNALS_MODULES_LIST.keys())
    {
	QLibrary *tmp_library = EXTERNALS_MODULES_LIST.value(clientModule);
	if (tmp_library)
	{
	    tmp_destroy_module_instance_fct = (destroy_module_instance_fct) tmp_library->resolve(MODULE_DESTROY_INSTANCE_ENTRY_NAME);

	    if (tmp_destroy_module_instance_fct != NULL)
	    {
		(*tmp_destroy_module_instance_fct)(clientModule);
	    }

	    tmp_library->unload();
	    delete tmp_library;
	}
    }


    return tmp_app_return;
}


