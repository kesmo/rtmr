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

#include "form_projects_reports.h"
#include "ui_Form_Projects_Reports.h"
#include "session.h"

Form_Projects_Reports::Form_Projects_Reports(QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Projects_Reports)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(_m_ui->radio_histogramme, SIGNAL(clicked()), this, SLOT(updateGraph()));
    connect(_m_ui->radio_polygone, SIGNAL(clicked()), this, SLOT(updateGraph()));
    connect(_m_ui->radio_pie, SIGNAL(clicked()), this, SLOT(updateGraph()));

    connect(_m_ui->graph_title, SIGNAL(textChanged(const QString &)), this, SLOT(updateGraph()));
    connect(_m_ui->title_color_button, SIGNAL(clicked()), this, SLOT(setTitleColor()));
    connect(_m_ui->inclinaison_X, SIGNAL(valueChanged(int)), this, SLOT(updateGraph()));
    connect(_m_ui->inclinaison_Y, SIGNAL(valueChanged(int)), this, SLOT(updateGraph()));
    connect(_m_ui->cheese_portions_space, SIGNAL(valueChanged(int)), this, SLOT(updateGraph()));
    connect(_m_ui->transparency, SIGNAL(valueChanged(int)), this, SLOT(updateGraph()));

    connect(_m_ui->show_legend, SIGNAL(clicked()), this, SLOT(updateGraph()));
    connect(_m_ui->raised, SIGNAL(clicked()), this, SLOT(updateGraph()));
    connect(_m_ui->stack, SIGNAL(clicked()), this, SLOT(updateGraph()));

    connect(_m_ui->tests_by_projects, SIGNAL(clicked()), this, SLOT(draw()));
    connect(_m_ui->tests_executions_by_projects, SIGNAL(clicked()), this, SLOT(draw()));
    connect(_m_ui->tests_executions_ok_by_projects, SIGNAL(clicked()), this, SLOT(draw()));
    connect(_m_ui->tests_executions_ko_by_projects, SIGNAL(clicked()), this, SLOT(draw()));
    connect(_m_ui->all_tests_executions_by_projects, SIGNAL(clicked()), this, SLOT(draw()));

    connect(_m_ui->save_button, SIGNAL(clicked()), this, SLOT(saveAs()));

    init();
}

Form_Projects_Reports::~Form_Projects_Reports()
{
    delete _m_chart;
    delete _m_ui;
    qDeleteAll(_m_chart_data);
}

void Form_Projects_Reports::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        _m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Form_Projects_Reports::init()
{
    QPixmap tmp_title_color_pixmap(16, 16);

    _m_chart = new Chart(800, 600);
    _m_chart->setRenderingForQuality();

    tmp_title_color_pixmap.fill(Chart::jsColorToJavaColor(_m_chart->getTitleColor()));
    _m_ui->title_color_button->setIcon(tmp_title_color_pixmap);

    draw();
}


TrinomeArray Form_Projects_Reports::loadDatas()
{
    char				***tmp_first_result = NULL;
    unsigned long       tmp_first_rows_count = 0;
    unsigned long       tmp_first_columns_count = 0;

    char				***tmp_second_result = NULL;
    unsigned long       tmp_second_rows_count = 0;
    unsigned long       tmp_second_columns_count = 0;

    char				***tmp_third_result = NULL;
    unsigned long       tmp_third_rows_count = 0;
    unsigned long       tmp_third_columns_count = 0;

    net_session		*tmp_session = Session::instance()->getClientSession();

    qDeleteAll(_m_chart_data);
    _m_chart_data.clear();

    if (_m_ui->tests_by_projects->isChecked())
    {
    	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
    		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null) as x on"
    		" (projects_table.project_id=x.project_id)"
    		" group by projects_table.short_name order by projects_table.short_name;");
		tmp_first_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_first_rows_count, &tmp_first_columns_count);

	for (unsigned long tmp_index = 0; tmp_index < tmp_first_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_first_result[tmp_index][0]), new Double(atol(tmp_first_result[tmp_index][1]))));
	}
	cl_free_rows_columns_array(&tmp_first_result, tmp_first_rows_count, tmp_first_columns_count);
    }
    else if (_m_ui->tests_executions_by_projects->isChecked())
    {
    	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
    		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
    		" and tests_table.test_id in (select test_id"
    		" from executions_tests_table)) as x on"
    		" (projects_table.project_id=x.project_id)"
    		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_first_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_first_rows_count, &tmp_first_columns_count);

	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
		" and tests_table.test_id not in (select test_id"
		" from executions_tests_table)) as x on"
		" (projects_table.project_id=x.project_id)"
		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_second_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_second_rows_count, &tmp_second_columns_count);

	for (unsigned long tmp_index = 0; tmp_index < tmp_first_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_first_result[tmp_index][0]),
                                              new Double(atol(tmp_first_result[tmp_index][1])),
                                              new String(TR_CUSTOM_MESSAGE("Exécuté"))));
	}

	for (unsigned long tmp_index = 0; tmp_index < tmp_second_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_second_result[tmp_index][0]),
                                              new Double(atol(tmp_second_result[tmp_index][1])),
                                              new String(TR_CUSTOM_MESSAGE("Non exécuté"))));
	}

	cl_free_rows_columns_array(&tmp_second_result, tmp_second_rows_count, tmp_second_columns_count);
	cl_free_rows_columns_array(&tmp_first_result, tmp_first_rows_count, tmp_first_columns_count);
    }
    else if (_m_ui->tests_executions_ok_by_projects->isChecked())
    {
    	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
    		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
    		" and tests_table.test_id in (select test_id"
    		" from executions_tests_table where result_id='0')) as x on"
    		" (projects_table.project_id=x.project_id)"
    		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_first_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_first_rows_count, &tmp_first_columns_count);

	for (unsigned long tmp_index = 0; tmp_index < tmp_first_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_first_result[tmp_index][0]),
                                              new Double(atol(tmp_first_result[tmp_index][1]))));
	}
	cl_free_rows_columns_array(&tmp_first_result, tmp_first_rows_count, tmp_first_columns_count);
    }
    else if (_m_ui->tests_executions_ko_by_projects->isChecked())
    {
    	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
    		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
    		" and tests_table.test_id in (select test_id"
    		" from executions_tests_table where result_id='1')) as x on"
    		" (projects_table.project_id=x.project_id)"
    		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_first_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_first_rows_count, &tmp_first_columns_count);

	for (unsigned long tmp_index = 0; tmp_index < tmp_first_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_first_result[tmp_index][0]),
                                              new Double(atol(tmp_first_result[tmp_index][1]))));
	}
	cl_free_rows_columns_array(&tmp_first_result, tmp_first_rows_count, tmp_first_columns_count);
    }
    else if (_m_ui->all_tests_executions_by_projects->isChecked())
    {
    	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
    		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
    		" and tests_table.test_id in (select test_id"
    		" from executions_tests_table where result_id='0')) as x on"
    		" (projects_table.project_id=x.project_id)"
    		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_first_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_first_rows_count, &tmp_first_columns_count);

	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
		" and tests_table.test_id in (select test_id"
		" from executions_tests_table where result_id='1')) as x on"
		" (projects_table.project_id=x.project_id)"
		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_second_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_second_rows_count, &tmp_second_columns_count);

	sprintf(tmp_session->m_last_query, "select projects_table.short_name, count(x.project_id) from projects_table"
		" left outer join (select tests_table.project_id from tests_table where tests_table.original_test_id is null"
		" and tests_table.test_id in (select test_id"
		" from executions_tests_table where result_id='2')) as x on"
		" (projects_table.project_id=x.project_id)"
		" group by projects_table.short_name order by projects_table.short_name;");
	tmp_third_result = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_third_rows_count, &tmp_third_columns_count);

	for (unsigned long tmp_index = 0; tmp_index < tmp_first_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_first_result[tmp_index][0]),
                                              new Double(atol(tmp_first_result[tmp_index][1])),
                                              new String(TR_CUSTOM_MESSAGE("KO"))));
	}

	for (unsigned long tmp_index = 0; tmp_index < tmp_second_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_second_result[tmp_index][0]),
                                              new Double(atol(tmp_second_result[tmp_index][1])),
                                              new String(TR_CUSTOM_MESSAGE("KO"))));
	}

	for (unsigned long tmp_index = 0; tmp_index < tmp_third_rows_count; tmp_index++)
	{
        _m_chart_data.append(new Trinome(new String(tmp_third_result[tmp_index][0]),
                                              new Double(atol(tmp_third_result[tmp_index][1])),
                                              new String(TR_CUSTOM_MESSAGE("Non passé"))));
	}
	cl_free_rows_columns_array(&tmp_third_result, tmp_third_rows_count, tmp_third_columns_count);
	cl_free_rows_columns_array(&tmp_second_result, tmp_second_rows_count, tmp_second_columns_count);
	cl_free_rows_columns_array(&tmp_first_result, tmp_first_rows_count, tmp_first_columns_count);
    }

    return TrinomeArray(_m_chart_data);
}


void Form_Projects_Reports::draw()
{
    _m_chart->setData(loadDatas());
    updateGraph();
}

void Form_Projects_Reports::updateGraph()
{
    _m_chart->setTitle(_m_ui->graph_title->text());

    _m_chart->m_raised_histogram_indic = _m_ui->raised->isChecked();
    _m_chart->m_show_legend = _m_ui->show_legend->isChecked();
    _m_chart->m_stack_histogram_indic  = _m_ui->stack->isChecked();

    _m_chart->setInclineX(_m_ui->inclinaison_X->value());
    _m_chart->setInclineY(_m_ui->inclinaison_Y->value());
    _m_chart->setCheesePortionSpace(_m_ui->cheese_portions_space->value());
    _m_chart->setGraphOpacity(100 - _m_ui->transparency->value());

    _m_chart->clear();

    if (_m_ui->radio_histogramme->isChecked())
        _m_chart->drawGraph(Chart::Histogramme);
    else if (_m_ui->radio_polygone->isChecked())
        _m_chart->drawGraph(Chart::Polygone);
    else if (_m_ui->radio_pie->isChecked())
        _m_chart->drawGraph(Chart::Camembert);

    _m_ui->pixmap->setPixmap(*_m_chart->getImage());
    _m_ui->pixmap->update();
}


void Form_Projects_Reports::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Image PNG", QString(), "*.png");
    if (!fileName.isEmpty())
    {
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".png");

        _m_chart->getImage()->save(fileName, 0, -1);
    }
}


void Form_Projects_Reports::setTitleColor()
{
    QPixmap tmp_title_color_pixmap(16, 16);
    QColor	tmp_color = QColorDialog::getColor(Chart::jsColorToJavaColor(_m_chart->getTitleColor()), this);
    if (!tmp_color.isValid())
        return;

    tmp_title_color_pixmap.fill(tmp_color);
    _m_ui->title_color_button->setIcon(tmp_title_color_pixmap);

    _m_chart->setTitleColor(Chart::javaColorToJsColor(tmp_color));

    updateGraph();
}

