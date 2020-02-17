#include "OpusConverter.h"
#include "Utils/MetaData/MetaData.h"

struct OpusConverter::Private
{
	bool cbr;

	Private(bool cbr) :
		cbr(cbr)
	{}
};

OpusConverter::OpusConverter(bool cbr, int quality, QObject* parent) :
	Converter(quality, parent)
{
	m = Pimpl::make<Private>(cbr);
}

OpusConverter::~OpusConverter() = default;

QStringList OpusConverter::supportedInputFormats() const
{
	return {"flac", "wav"};
}

QString OpusConverter::binary() const
{
	return "opusenc";
}

QStringList OpusConverter::processEntry(const MetaData& md) const
{
	QStringList ret
	{
		QString("--title"), QString("%1").arg(md.title()).toUtf8().data(),
		QString("--artist"), QString("%1").arg(md.artist().toUtf8().data()),
		QString("--album"), QString("%1").arg(md.album()).toUtf8().data(),
		//QString("--comment"), QString("%1").arg(md.comment()).toUtf8().data(),
		QString("--bitrate"), QString("%1.000").arg(quality())
	};

	if(m->cbr)
	{
		ret << "--hard-cbr";
	}

	else
	{
		ret << "--vbr";
	}

	ret << QStringList
	{
		QString("%1").arg(md.filepath()),
		QString("%1").arg(targetFile(md))
	};

	return ret;
}

QString OpusConverter::extension() const
{
	return "opus";
}
