#include "FileScanner.h"

#include "Components/Directories/DirectoryReader.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>


namespace PlaylistNs=::Playlist;
using PlaylistNs::FileScanner;

struct FileScanner::Private
{
	QStringList		paths;
	MetaDataList	metadata;
	int				playlist_id;

	Private(const QStringList& paths, int playlist_id) :
		paths(paths),
		playlist_id(playlist_id)
	{}
};

FileScanner::FileScanner(const QStringList& paths, int playlist_id) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>(paths, playlist_id);
}
FileScanner::~FileScanner() = default;

void FileScanner::start()
{
	DirectoryReader dr(Util::soundfile_extensions());
	m->metadata = dr.scan_metadata(m->paths);
	m->metadata.sort(Library::SortOrder::TrackAlbumArtistAsc);

	emit sig_finished();
}

MetaDataList FileScanner::metadata() const
{
	return m->metadata;
}

int FileScanner::playlist_id() const
{
	return m->playlist_id;
}
