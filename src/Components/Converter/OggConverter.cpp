#include "OggConverter.h"
#include "Utils/MetaData/MetaData.h"

#include <QStringList>

OggConverter::OggConverter(int quality, QObject* parent) :
	Converter(quality, parent)
{}

OggConverter::~OggConverter() {}

QStringList OggConverter::supported_input_formats() const
{
	return {"flac", "wav"};
}

QString OggConverter::binary() const
{
	return "oggenc";
}

QStringList OggConverter::process_entry(const MetaData& md) const
{
	QStringList ret
	{
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
