#include "PlaylistTestUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QString>

MetaDataList Test::Playlist::createTrackList(int min, int max)
{
	MetaDataList v_md;
	for(int i=min; i<max; i++)
	{
		MetaData md;
		QString p = QString("https://www.bla.com/path/to/%1.mp3").arg(i);

		md.setId(i);
		md.setFilepath(p);
		md.setDurationMs(i * 10000);

		v_md << md;
	}

	return v_md;
}
