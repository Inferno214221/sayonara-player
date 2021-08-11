#include "MetaDataScanner.h"

#include "Database/Connector.h"

#include "Utils/DirectoryReader.h"
#include "Utils/Utils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>
#include <QDir>

using Directory::MetaDataScanner;

struct MetaDataScanner::Private
{
	QStringList files;
	MetaDataList tracks;

	bool recursive;

	Private(const QStringList& files, bool recursive) :
		files(files),
		recursive(recursive) {}
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
	if(!m->recursive)
	{
		m->tracks.clear();

		const auto extensions = QStringList() << Util::soundfileExtensions();
		for(const auto& path : m->files)
		{
			emit sigCurrentProcessedPathChanged(path);

			const auto files = DirectoryReader::scanFilesInDirectory(QDir(path), extensions);
			m->tracks << DirectoryReader::scanMetadata(files);
		}
	}

	else
	{
		m->tracks = DirectoryReader::scanMetadata(m->files);
	}

	emit sigFinished();
}

MetaDataList MetaDataScanner::metadata() const { return m->tracks; }

QStringList MetaDataScanner::files() const { return m->files; }
