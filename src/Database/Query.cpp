/* Query.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database/Query.h"
#include "Database/Module.h"
#include "Utils/Logger/Logger.h"

#include <QSqlDriver>
#include <cmath>

//#define DB_DEBUG

using DB::Query;
using DB::Module;

struct Query::Private
{
	QString		query_string;
	bool		success;

	Private() :
		success(false)
	{}
};

Query::Query(const QString& connection_name, DbId db_id) :
	QSqlQuery(Module(connection_name, db_id).db())
{
	m = Pimpl::make<Private>();
}


Query::Query(const Module* module) :
	QSqlQuery(module->db())
{
	m = Pimpl::make<Private>();
}


Query::Query(QSqlDatabase db) :
	QSqlQuery(db)
{
	m = Pimpl::make<Private>();
}


Query::Query(const Query& other) :
	QSqlQuery(other)
{
	m = Pimpl::make<Private>();
	m->query_string = other.m->query_string;
	m->success = other.m->success;
}

DB::Query& Query::operator=(const DB::Query& other)
{
	QSqlQuery::operator =(other);
	m->query_string = other.m->query_string;
	m->success = other.m->success;

	return *this;
}

Query::~Query()
{
	this->clear();
}

bool Query::prepare(const QString& query)
{
	m->query_string = query;

	return QSqlQuery::prepare(query);
}

void Query::bindValue(const QString& placeholder, const QVariant& val, QSql::ParamType param_type )
{
	QString replace_str = QString("'") + val.toString() + "'";

	m->query_string.replace(placeholder + " ", replace_str + " ");
	m->query_string.replace(placeholder + ",", replace_str + ",");
	m->query_string.replace(placeholder + ";", replace_str + ";");
	m->query_string.replace(placeholder + ")", replace_str + ")");

	QSqlQuery::bindValue(placeholder, val, param_type);
}

#undef DB_DEBUG
#ifdef DB_DEBUG
	#include <QTime>
	#include <QHash>
	static int n_queries=0;
	static QHash<QString, int> query_map;
#endif

bool Query::exec()
{
#ifdef DB_DEBUG
	QTime timer;
	timer.start();
	n_queries++;
	int val = 1;
	int n_calls = 0;
	if(query_map.contains(m->query_string)){
		val = query_map[m->query_string] + 1;
		n_calls = val;
	}

	query_map[m->query_string] = val;

#endif

	m->success = QSqlQuery::exec();

#ifdef DB_DEBUG
	sp_log(Log::Debug, this) << QString("(%1) ").arg(n_queries)
							 << m->query_string << ": "
							 << timer.elapsed() << "ms";

	if(n_calls > 1){
		sp_log(Log::Debug, this) << QString("Called %1 times").arg(val);
	}


#endif

	return m->success;
}

void Query::set_error(bool b)
{
	m->success = (!b);
}

bool Query::has_error() const
{
	return (m->success == false);
}

QString Query::get_query_string() const
{
	QString str = m->query_string;
	str.prepend("\n");
	str.replace("SELECT ", "SELECT\n", Qt::CaseInsensitive);
	str.replace("FROM", "\nFROM", Qt::CaseInsensitive);
	str.replace(",", ",\n", Qt::CaseInsensitive);
	str.replace("INNER JOIN", "\nINNER JOIN", Qt::CaseInsensitive);
	str.replace("LEFT OUTER JOIN", "\nLEFT OUTER JOIN", Qt::CaseInsensitive);
	str.replace("UNION", "\nUNION", Qt::CaseInsensitive);
	str.replace("GROUP BY", "\nGROUP BY", Qt::CaseInsensitive);
	str.replace("ORDER BY", "\nORDER BY", Qt::CaseInsensitive);
	str.replace("WHERE", "\nWHERE", Qt::CaseInsensitive);
	str.replace("(", "\n(\n");
	str.replace(")", "\n)\n");

	int idx = str.indexOf("(");
	while(idx >= 0){
		int idx_close = str.indexOf(")", idx);
		int nl = str.indexOf("\n", idx);
		while(nl > 0 && nl < idx_close)
		{
			str.insert(nl + 1, '\t');
			nl = str.indexOf("\n", nl + 2);
		}

		idx = str.indexOf("(", idx_close);
	}

	while(str.contains("\n ")){
		str.replace("\n ", "\n");
	}

	while(str.contains(", ")){
		str.replace(", ", ",");
	}

	while(str.contains(" ,")){
		str.replace(" ,", ",");
	}

	while(str.contains("  ")){
		str.replace("  ", " ");
	}

	while(str.contains("\n\n")){
		str.replace("\n\n", "\n");
	}

	return str;
}

void Query::show_query() const
{
	sp_log(Log::Debug, this) << get_query_string();
}

void Query::show_error(const QString& err_msg) const
{
	sp_log(Log::Error, this) << "SQL ERROR: " << err_msg << ": " << (int) this->lastError().type();

	QSqlError e = this->lastError();
	if(!e.text().isEmpty()){
		sp_log(Log::Error, this) << e.text();
	}

	if(!e.driverText().isEmpty()) {
		sp_log(Log::Error, this) << e.driverText();
	}

	if(!e.databaseText().isEmpty()){
		sp_log(Log::Error, this) << e.databaseText();
	}

#ifdef DEBUG
	sp_log(Log::Error, this) << m->query_string;
#endif

	sp_log(Log::Error, this) << this->get_query_string();
}

size_t Query::fetched_rows()
{
	int last_pos = this->at();

	this->last();
	int rows = this->at() + 1;
	this->seek(last_pos);

	if(rows < 0){
		return 0;
	}

	return size_t(rows);
}
