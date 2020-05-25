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
	QStringList files;
	QStringList extensions;
	MetaDataList tracks;

	void* data = nullptr;

	bool recursive;
	bool scanAudioFiles;
	bool scanPlaylistFiles;

	Private(const QStringList& files, bool recursive) :
		files(files),
		recursive(recursive),
		scanAudioFiles(true),
		scanPlaylistFiles(false)
	{}
};

MetaDataScanner::~MetaDataScanner()
{
	DB::Connector::instance()->closeDatabase();
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
	if(m->scanAudioFiles)
	{
		extensions << Util::soundfileExtensions();
	}

	if(m->scanPlaylistFiles)
	{
		extensions << Util::playlistExtensions();
	}

	if(extensions.isEmpty())
	{
		emit sigFinished();
		return;
	}

	reader.setFilter(extensions);

	if(!m->recursive)
	{
		m->tracks.clear();
		for(const QString& path : m->files)
		{
			emit sigCurrentProcessedPathChanged(path);

			QStringList files;
			reader.scanFiles(QDir(path), files);
			m->tracks << reader.scanMetadata(files);
		}
	}

	else
	{
		m->tracks = reader.scanMetadata(m->files);
	}

	m->tracks.sort(Library::SortOrder::TrackAlbumArtistAsc);

	emit sigFinished();
}

MetaDataList MetaDataScanner::metadata() const
{
	return m->tracks;
}

QStringList MetaDataScanner::files() const
{
	return m->files;
}

void MetaDataScanner::setData(void* data_object)
{
	m->data = data_object;
}

void* MetaDataScanner::data() const
{
	return m->data;
}
