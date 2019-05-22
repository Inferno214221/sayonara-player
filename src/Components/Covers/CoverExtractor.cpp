#include "CoverExtractor.h"
#include <QString>
#include <QPixmap>

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/TaggingCover.h"

namespace FileUtils=::Util::File;

struct Cover::Extractor::Private
{
	QPixmap pixmap;
	QString filepath;

	Private(const QString& filepath) : filepath(filepath) {}
};

Cover::Extractor::Extractor(const QString& filepath, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(filepath);
}

Cover::Extractor::~Extractor() {}

QPixmap Cover::Extractor::pixmap()
{
	return m->pixmap;
}

void Cover::Extractor::start()
{
	m->pixmap = QPixmap();

	if(FileUtils::exists(m->filepath)){
		sp_log(Log::Develop, this) << "Extractor: extract cover out of " << m->filepath;
		m->pixmap = Tagging::Covers::extract_cover(m->filepath);
	}

	sp_log(Log::Develop, this) << "Extractor: emit finished";
	emit sig_finished();
}
