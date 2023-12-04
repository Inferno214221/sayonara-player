#include "MetaDataScanner.h"

#include "Database/Connector.h"

#include "Utils/DirectoryReader.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QDir>

using Directory::MetaDataScanner;

struct MetaDataScanner::Private
{
	QStringList files;
	MetaDataList tracks;

	bool recursive;

	Private(QStringList files, bool recursive) :
		files(std::move(files)),
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
	const auto fileSystem = Util::FileSystem::create();
	const auto tagReader = Tagging::TagReader::create();
	const auto directoryReader = Util::DirectoryReader::create(fileSystem, tagReader);
	if(!m->recursive)
	{
		m->tracks.clear();

		const auto extensions = QStringList() << Util::soundfileExtensions();
		for(const auto& path: m->files)
		{
			emit sigCurrentProcessedPathChanged(path);

			const auto files = directoryReader->scanFilesInDirectory(QDir(path), extensions);
			m->tracks << directoryReader->scanMetadata(files);
		}
	}

	else
	{
		m->tracks = directoryReader->scanMetadata(m->files);
	}

	emit sigFinished();
}

MetaDataList MetaDataScanner::metadata() const { return m->tracks; }

QStringList MetaDataScanner::files() const { return m->files; }
