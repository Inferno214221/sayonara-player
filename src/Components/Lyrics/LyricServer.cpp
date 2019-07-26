#include "LyricServer.h"

bool Lyrics::Server::can_fetch_directly() const
{
	return true;
}

bool Lyrics::Server::can_search() const
{
	return false;
}

QString Lyrics::Server::search_address(QString artist, QString title) const
{
	Q_UNUSED(artist)
	Q_UNUSED(title)
	return QString();
}

QString Lyrics::Server::parse_search_result(const QString& search_result)
{
	Q_UNUSED(search_result)
	return QString();
}

QString Lyrics::Server::apply_replacements(const QString& str, const Lyrics::Server::Replacements& replacements)
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

QString Lyrics::Server::apply_replacements(const QString& str) const
{
	return apply_replacements(str, this->replacements());
}


QString Lyrics::SearchableServer::address() const
{
	return QString();
}

Lyrics::Server::Replacements Lyrics::SearchableServer::replacements() const
{
	return Lyrics::Server::Replacements();
}

QString Lyrics::SearchableServer::call_policy() const
{
	return QString();
}

bool Lyrics::SearchableServer::can_search() const
{
	return true;
}

