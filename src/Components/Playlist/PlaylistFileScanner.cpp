#include "PlaylistFileScanner.h"

#include "Components/Directories/DirectoryReader.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>
#include <QThread>

namespace PlaylistNs=::Playlist;
using PlaylistNs::FileScanner;

struct PlaylistNs::FileScanner::Private
{
	QStringList		paths;
	MetaDataList	metadata;
	int				playlist_id;
	int				target_row_index;

	Private(int playlist_id, const QStringList& paths, int target_row_index) :
		paths(paths),
		playlist_id(playlist_id),
		target_row_index(target_row_index)
	{}
};

PlaylistNs::FileScanner::FileScanner(int playlist_id, const QStringList& paths, int target_row_index) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>(playlist_id, paths, target_row_index);
}

PlaylistNs::FileScanner::~FileScanner() = default;

void PlaylistNs::FileScanner::start()
{
	DirectoryReader dr(Util::soundfile_extensions());

	m->metadata = dr.scan_metadata(m->paths);
	m->metadata.sort(Library::SortOrder::TrackAlbumArtistAsc);

	Util::sleep_ms(10000);

	emit sig_finished();
}

MetaDataList PlaylistNs::FileScanner::metadata() const
{
	return m->metadata;
}

int PlaylistNs::FileScanner::playlist_id() const
{
	return m->playlist_id;
}

int FileScanner::target_row_index() const
{
	return m->target_row_index;
}
