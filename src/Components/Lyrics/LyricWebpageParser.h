#ifndef LYRICWEBPAGEPARSER_H
#define LYRICWEBPAGEPARSER_H

#include <QMap>
#include <QString>
#include <QByteArray>

namespace Lyrics
{
	class Server;
	namespace WebpageParser
	{
		QString parse_webpage(const QByteArray& data, const QMap<QString, QString>& regex_conversions, Server* server);
	}
}

#endif // LYRICWEBPAGEPARSER_H
