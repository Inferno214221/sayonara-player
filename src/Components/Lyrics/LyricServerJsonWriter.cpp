/* LyricServerJsonWriter.cpp */

/* Copyright (C) 2011-2020 Lucio Carreras
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



#include "LyricServer.h"
#include "LyricServerJsonWriter.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

using Lyrics::Server;

QJsonObject Lyrics::ServerJsonWriter::to_json(Lyrics::Server* server)
{
	QJsonObject object;

	object.insert("name", QJsonValue(server->name()));
	object.insert("address", QJsonValue(server->address()));
	object.insert("direct_url_template", QJsonValue(server->direct_url_template()));
	object.insert("is_start_tag_included", QJsonValue(server->is_start_tag_included()));
	object.insert("is_end_tag_included", QJsonValue(server->is_end_tag_included()));
	object.insert("is_numeric", QJsonValue(server->is_numeric()));
	object.insert("is_lowercase", QJsonValue(server->is_lowercase()));
	object.insert("error_string", QJsonValue(server->error_string()));

	QJsonArray arr_replacements;
	auto replacements = server->replacements();
	for(const Server::Replacement& replacement : replacements)
	{
		QJsonObject replacement_object;
		replacement_object.insert("replacement_from", QJsonValue(replacement.first));
		replacement_object.insert("replacement_to", QJsonValue(replacement.second));

		arr_replacements.append(QJsonValue(replacement_object));
	}

	object.insert("replacements", QJsonValue(arr_replacements));

	QJsonArray arr_start_end_tags;
	auto start_end_tags = server->start_end_tag();
	for(const Server::StartEndTag& start_end_tag : start_end_tags)
	{
		QJsonObject start_end_tag_object;
		start_end_tag_object.insert("start_tag", QJsonValue(start_end_tag.first));
		start_end_tag_object.insert("end_tag", QJsonValue(start_end_tag.second));

		arr_start_end_tags.append(QJsonValue(start_end_tag_object));
	}

	object.insert("start_end_tags", QJsonValue(arr_start_end_tags));

	object.insert("search_result_regex", QJsonValue(server->search_result_regex()));
	object.insert("search_result_url_template", QJsonValue(server->search_result_url_template()));
	object.insert("search_url_template", QJsonValue(server->search_url_template()));

	return object;
}


Lyrics::Server* Lyrics::ServerJsonReader::from_json(const QJsonObject& json)
{
	Server* server = new Server();

	for(auto it=json.begin(); it != json.end(); it++)
	{
		QString key = it.key();
		QJsonValue val = *it;

		if(key == "name")
		{
			server->set_name(val.toString());
		}

		else if(key == "address")
		{
			server->set_address(val.toString());
		}

		else if(key == "replacements")
		{
			Server::Replacements replacements;
			QJsonArray arr = val.toArray();
			for(auto arr_it=arr.begin(); arr_it != arr.end(); arr_it++)
			{

				Server::Replacement replacement;

				QJsonObject obj = (*arr_it).toObject();
				for(auto obj_it=obj.begin(); obj_it != obj.end(); obj_it++)
				{
					if(obj_it.key() == "replacement_from")
					{
						replacement.first = (*obj_it).toString();
					}

					else if(obj_it.key() == "replacement_to")
					{
						replacement.second = (*obj_it).toString();
					}
				}

				if(!replacement.first.isEmpty()){
					replacements << replacement;
				}
			}

			server->set_replacements(replacements);
		}

		else if(key == "direct_url_template")
		{
			server->set_direct_url_template(val.toString());
		}

		else if(key == "start_end_tags")
		{
			Server::StartEndTags start_end_tags;
			QJsonArray arr = val.toArray();
			for(auto arr_it=arr.begin(); arr_it != arr.end(); arr_it++)
			{
				Server::StartEndTag start_end_tag;

				QJsonObject obj = (*arr_it).toObject();
				for(auto obj_it=obj.begin(); obj_it != obj.end(); obj_it++)
				{
					if(obj_it.key() == "start_tag")
					{
						start_end_tag.first = (*obj_it).toString();
					}

					else if(obj_it.key() == "end_tag")
					{
						start_end_tag.second = (*obj_it).toString();
					}
				}

				if(!start_end_tag.first.isEmpty() && !start_end_tag.second.isEmpty()){
					start_end_tags << start_end_tag;
				}
			}

			server->set_start_end_tag(start_end_tags);
		}

		else if(key == "is_start_tag_included")
		{
			server->set_is_start_tag_included(val.toBool());
		}

		else if(key == "is_end_tag_included")
		{
			server->set_is_end_tag_included(val.toBool());
		}

		else if(key == "is_numeric")
		{
			server->set_is_numeric(val.toBool());
		}

		else if(key == "is_lowercase")
		{
			server->set_is_lowercase(val.toBool());
		}

		else if(key == "error_string")
		{
			server->set_error_string(val.toString());
		}

		else if(key == "search_result_regex")
		{
			server->set_search_result_regex(val.toString());
		}

		else if(key == "search_result_url_template")
		{
			server->set_search_result_url_template(val.toString());
		}

		else if(key == "search_url_template")
		{
			server->set_search_url_template(val.toString());
		}
	}

	if(server->name().isEmpty() || server->address().isEmpty())
	{
		delete server;
		return nullptr;
	}

	return server;
}


QList<Lyrics::Server*> Lyrics::ServerJsonReader::parse_json_file(const QString& filename)
{
	QList<Server*> ret;

	QFile f(filename);
	f.open(QFile::ReadOnly);
	QByteArray data = f.readAll();
	f.close();

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if(doc.isArray())
	{
		QJsonArray arr = doc.array();
		for(auto it=arr.begin(); it != arr.end(); it++)
		{
			Server* server = from_json( (*it).toObject() );
			if(server){
				ret << server;
			}
		}
	}

	else if(doc.isObject())
	{
		Server* server = from_json( doc.object() );
		if(server){
			ret << server;
		}
	}

	return ret;
}
