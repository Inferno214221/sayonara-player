#include "LyricServer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using namespace Lyrics;

struct Server::Private
{
	QString name;
	QString address;
	Server::Replacements replacements;
	QString direct_url_template;
	Server::StartEndTags start_end_tag;
	bool is_start_tag_included=false;
	bool is_end_tag_included=false;
	bool is_numeric=false;
	bool is_lowercase=false;
	QString error_string;

	QString search_result_regex;
	QString search_result_url_template;
	QString search_url_template;
};

bool Server::can_fetch_directly() const
{
	return !(m->address.isEmpty() || m->direct_url_template.isEmpty() || m->start_end_tag.isEmpty());
}

bool Server::can_search() const
{
	return !(m->address.isEmpty() || m->search_result_regex.isEmpty() || m->search_result_url_template.isEmpty() || m->search_url_template.isEmpty());
}

QString Server::name() const
{
	return m->name;
}

void Server::set_name(const QString& name)
{
	m->name = name;
}

QString Server::address() const
{
	return m->address;
}

void Server::set_address(const QString& address)
{
	m->address = address;
}

Server::Replacements Server::replacements() const
{
	return m->replacements;
}

void Server::set_replacements(const Server::Replacements& replacements)
{
	m->replacements = replacements;
}

QString Server::direct_url_template() const
{
	return m->direct_url_template;
}

void Server::set_direct_url_template(const QString& direct_url_template)
{
	m->direct_url_template = direct_url_template;
}

Server::StartEndTags Server::start_end_tag() const
{
	return m->start_end_tag;
}

void Server::set_start_end_tag(const StartEndTags& start_end_tag)
{
	m->start_end_tag = start_end_tag;
}

bool Server::is_start_tag_included() const
{
	return m->is_start_tag_included;
}

void Server::set_is_start_tag_included(bool is_start_tag_included)
{
	m->is_start_tag_included = is_start_tag_included;
}

bool Server::is_end_tag_included() const
{
	return m->is_end_tag_included;
}

void Server::set_is_end_tag_included(bool is_end_tag_included)
{
	m->is_end_tag_included = is_end_tag_included;
}

bool Server::is_numeric() const
{
	return m->is_numeric;
}

void Server::set_is_numeric(bool is_numeric)
{
	m->is_numeric = is_numeric;
}

bool Server::is_lowercase() const
{
	return m->is_lowercase;
}

void Server::set_is_lowercase(bool is_lowercase)
{
	m->is_lowercase = is_lowercase;
}

QString Server::error_string() const
{
	return m->error_string;
}

void Server::set_error_string(const QString& error_string)
{
	m->error_string = error_string;
}

QString Server::search_result_regex() const
{
	return m->search_result_regex;
}

void Server::set_search_result_regex(const QString& search_result_regex)
{
	m->search_result_regex = search_result_regex;
}

QString Server::search_result_url_template() const
{
	return m->search_result_url_template;
}

void Server::set_search_result_url_template(const QString& search_result_url_template)
{
	m->search_result_url_template = search_result_url_template;
}

QString Server::search_url_template() const
{
	return m->search_url_template;
}

void Server::set_search_url_template(const QString& search_url_template)
{
	m->search_url_template = search_url_template;
}

QString Server::apply_replacements(const QString& str, const Server::Replacements& replacements)
{
	QString ret(str);

	for(const Server::Replacement& r : replacements)
	{
		while(ret.contains(r.first))
		{
			ret.replace(r.first, r.second);
		}
	}

	return ret;
}

QString Server::apply_replacements(const QString& str) const
{
	return apply_replacements(str, this->replacements());
}

QJsonObject Server::to_json()
{
	QJsonObject server;

	server.insert("name", QJsonValue(this->name()));
	server.insert("address", QJsonValue(this->address()));
	server.insert("direct_url_template", QJsonValue(this->direct_url_template()));
	server.insert("is_start_tag_included", QJsonValue(this->is_start_tag_included()));
	server.insert("is_end_tag_included", QJsonValue(this->is_end_tag_included()));
	server.insert("is_numeric", QJsonValue(this->is_numeric()));
	server.insert("is_lowercase", QJsonValue(this->is_lowercase()));
	server.insert("error_string", QJsonValue(this->error_string()));

	QJsonArray arr_replacements;
	auto replacements = this->replacements();
	for(const Server::Replacement& replacement : replacements)
	{
		QJsonObject replacement_object;
		replacement_object.insert("replacement_from", QJsonValue(replacement.first));
		replacement_object.insert("replacement_to", QJsonValue(replacement.second));

		arr_replacements.append(QJsonValue(replacement_object));
	}

	server.insert("replacements", QJsonValue(arr_replacements));

	QJsonArray arr_start_end_tags;
	auto start_end_tags = this->start_end_tag();
	for(const Server::StartEndTag& start_end_tag : start_end_tags)
	{
		QJsonObject start_end_tag_object;
		start_end_tag_object.insert("start_tag", QJsonValue(start_end_tag.first));
		start_end_tag_object.insert("end_tag", QJsonValue(start_end_tag.second));

		arr_start_end_tags.append(QJsonValue(start_end_tag_object));
	}

	server.insert("start_end_tags", QJsonValue(arr_start_end_tags));

	server.insert("search_result_regex", QJsonValue(search_result_regex()));
	server.insert("search_result_url_template", QJsonValue(search_result_url_template()));
	server.insert("search_url_template", QJsonValue(search_url_template()));

	return server;
}

Server* Server::from_json(const QJsonObject& json)
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

Server::Server()
{
	m = Pimpl::make<Private>();
}

Server::~Server() = default;
