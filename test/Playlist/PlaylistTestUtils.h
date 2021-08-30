#ifndef PLAYLISTTESTUTILS_H
#define PLAYLISTTESTUTILS_H

#include <QList>

#include <utility>

class MetaData;
class MetaDataList;
class QString;

namespace Test
{
	namespace Playlist
	{
		MetaDataList createTrackList(int min, int max);

		using PathTrackMap = QList<std::pair<QString, MetaData>>;
		PathTrackMap createTrackFiles(const QString& basePath);
	}
}

#endif // PLAYLISTTESTUTILS_H
