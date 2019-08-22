#include "FileScanner.h"

#include "Components/Directories/DirectoryReader.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDir>

struct Tagging::FileScanner::Private
{
	QString path;
	MetaDataList metadata;

	Private(const QString& path) : path(path) {}
};

Tagging::FileScanner::FileScanner(const QString& path) :
	QObject(nullptr)
{
	m = Pimpl::make<Private>(path);
}

Tagging::FileScanner::~FileScanner() = default;

void Tagging::FileScanner::start()
{
	m->metadata.clear();

	QString dir, filename;
	Util::File::split_filename(m->path, dir, filename);
	DirectoryReader dr(Util::soundfile_extensions());

	QStringList files;
	dr.scan_files(QDir(dir), files);
	m->metadata = dr.scan_metadata(files);

	emit sig_finished();
}

MetaDataList Tagging::FileScanner::metadata() const
{
	return m->metadata;
}
