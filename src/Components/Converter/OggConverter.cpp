#include "OggConverter.h"
#include "Utils/MetaData/MetaData.h"

#include <QStringList>

OggConverter::OggConverter(const QString& target_dir, int num_processes, int quality, QObject* parent) :
	Converter(target_dir, num_processes, quality, parent)
{}

OggConverter::~OggConverter() {}

QStringList OggConverter::get_process_entry(const MetaData& md) const
{
	QStringList ret
	{
		QString("oggenc"),
		QString("-q"), QString("%1").arg(quality()),
		QString("-o"), QString("%1").arg(target_file(md)),
		QString("%1").arg(md.filepath())
	};

	return ret;
}

QString OggConverter::extension() const
{
	return QString("ogg");
}
