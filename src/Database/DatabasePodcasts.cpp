/* DatabasePodcasts.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Database/SayonaraQuery.h"
#include "Database/DatabasePodcasts.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Utils.h"

using DB::Podcasts;
using DB::Query;

Podcasts::Podcasts(const QSqlDatabase& db, DbId db_id) :
	DB::Module(db, db_id) {}

Podcasts::~Podcasts() {}

bool Podcasts::getAllPodcasts(QMap<QString, QString> & podcasts)
{
	podcasts.clear();

	Query q(this);
	q.prepare("SELECT name, url FROM savedpodcasts;");

	if (!q.exec()){
		q.show_error("Cannot fetch podcasts");
		return false;
	}

	while(q.next()) {
		QString name = q.value(0).toString();
		QString url = q.value(1).toString();

		podcasts[name] = url;
	}

	return true;
}


bool Podcasts::deletePodcast(const QString& name)
{
	Query q(this);

	q.prepare("DELETE FROM savedpodcasts WHERE name = :name;" );
	q.bindValue(":name", Util::cvt_not_null(name));

	if(!q.exec()) {
		q.show_error(QString("Could not delete podcast ") + name);
		return false;
	}

	return true;
}


bool Podcasts::addPodcast(const QString& name, const QString& url)
{
	Query q(this);
	q.prepare("INSERT INTO savedpodcasts (name, url) VALUES (:name, :url); " );
	q.bindValue(":name",	Util::cvt_not_null(name));
	q.bindValue(":url",		Util::cvt_not_null(url));

	if(!q.exec()) {
		sp_log(Log::Warning) << "Could not add podcast " << name << ", " << url;
		return false;
	}

	sp_log(Log::Info) << "podcast " << name << ", " << url << " successfully added";
	return true;
}


bool Podcasts::updatePodcastUrl(const QString& name, const QString& url)
{
	Query q(this);

	q.prepare("UPDATE savedpodcasts SET url=:url WHERE name=:name;");
	q.bindValue(":name",	Util::cvt_not_null(name));
	q.bindValue(":url",		Util::cvt_not_null(url));

	if(!q.exec()) {
		q.show_error(QString("Could not update podcast url ") + name);
		return false;
	}

	return true;
}



