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


QString Lyrics::SearchableServer::address() const
{
	return QString();
}

QMap<QString, QString> Lyrics::SearchableServer::replacements() const
{
	return QMap<QString, QString>();
}

QString Lyrics::SearchableServer::call_policy() const
{
	return QString();
}

bool Lyrics::SearchableServer::can_search() const
{
	return true;
}

