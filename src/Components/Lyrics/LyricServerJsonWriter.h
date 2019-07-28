#ifndef LYRICSERVERJSONWRITER_H
#define LYRICSERVERJSONWRITER_H

#include <QList>

class QJsonObject;

namespace Lyrics
{
	class Server;
	namespace ServerJsonWriter
	{
		QJsonObject to_json(Lyrics::Server* server);
	}

	namespace ServerJsonReader
	{
		Server* from_json(const QJsonObject& json);
		QList<Server*> parse_json_file(const QString& filename);
	}
}

#endif // LYRICSERVERJSONWRITER_H
