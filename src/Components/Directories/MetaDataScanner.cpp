#include "MetaDataScanner.h"

#include "Components/Directories/DirectoryReader.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Database/Connector.h"

#include <QStringList>
#include <QDir>

using Directory::MetaDataScanner;

struct MetaDataScanner::Private
{
	QStringList		files;
	QStringList		extensions;
	MetaDataList	tracks;

	void*			data=nullptr;

	bool			recursive;
	bool			scan_audio_files;
	bool			scan_playlist_files;

	Private(const QStringList& files, bool recursive) :
		files(files),
		recursive(recursive),
		scan_audio_files(true),
		scan_playlist_files(false)
	{}
};

MetaDataScanner::~MetaDataScanner()
{
	DB::Connector::instance()->close_db();
}

MetaDataScanner::MetaDataScanner(const QStringList& files, bool recursive, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(files, recursive);
}

void MetaDataScanner::start()
{
	DirectoryReader reader;
	QStringList extensions;
	if(m->scan_audio_files)
	{
		extensions << Util::soundfile_extensions();
	}

	if(m->scan_playlist_files)
	{
		extensions << Util::playlist_extensions();
	}

	if(extensions.isEmpty())
	{
		emit sig_finished();
		return;
	}

	reader.set_filter(extensions);

	if(!m->recursive)
	{
		m->tracks.clear();
		for(const QString& path : m->files)
		{
			emit sig_current_path(path);

			QStringList files;
			reader.scan_files(QDir(path), files);
			m->tracks << reader.scan_metadata(files);
		}
	}

	else
	{
		m->tracks = reader.scan_metadata(m->files);
	}

	m->tracks.sort(Library::SortOrder::TrackAlbumArtistAsc);

	emit sig_finished();
}

MetaDataList MetaDataScanner::metadata() const
{
	return m->tracks;
}

QStringList MetaDataScanner::files() const
{
	return m->files;
}

void MetaDataScanner::set_scan_audio_files(bool b)
{
	m->scan_audio_files = b;
}

void MetaDataScanner::set_scan_playlist_files(bool b)
{
	m->scan_playlist_files = b;
}

void MetaDataScanner::set_data(void* data_object)
{
	m->data = data_object;
}

void* MetaDataScanner::data() const
{
	return m->data;
}
