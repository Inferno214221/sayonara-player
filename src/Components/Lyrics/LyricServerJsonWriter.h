#ifndef LYRICSERVERJSONWRITER_H
#define LYRICSERVERJSONWRITER_H

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
		Lyrics::Server* from_json(const QJsonObject& object);
	}
}

#endif // LYRICSERVERJSONWRITER_H
