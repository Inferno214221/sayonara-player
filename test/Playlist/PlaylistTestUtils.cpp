#include "PlaylistTestUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QString>

MetaDataList Test::Playlist::create_v_md(int min, int max)
{
	MetaDataList v_md;
	for(int i=min; i<max; i++)
	{
		MetaData md;
		QString p = QString("https://www.bla.com/path/to/%1.mp3").arg(i);

		md.set_id(i);
		md.set_filepath(p);
		md.set_duration_ms(i * 10000);

		v_md << md;
	}

	return v_md;
}
