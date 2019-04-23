#include "FMStreamParser.h"
#include "Utils/Logger/Logger.h"
#include <QRegExp>

struct FMStreamParser::Private
{
	QList<RadioStation> stations;
};

static QString extract_regexp(const QString& data, const QString& re_str)
{
	QRegExp re(re_str);
	re.setMinimal(true);
	int idx = re.indexIn(data);
	if(idx >= 0) {
		return re.cap(1).trimmed();
	}

	return QString();
}

static void parse_stn1_block(const QString& data, RadioStation& station)
{
	station.name =				extract_regexp(data, "<h3>(.*)</h3>");
	station.image =				extract_regexp(data, "<img.+src=\"(.*)\"");
	station.location =			extract_regexp(data, "class=\"loc\".*>(.*)</span>");
	station.style =				extract_regexp(data, "class=\"sty\".*>(.*)</span>");
	station.frequency =			extract_regexp(data, "class=\"frq\".*>(.*)</span>");
	station.description =		extract_regexp(data, "class=\"desc\".*>(.*)</span>");
	station.short_description = extract_regexp(data, "class=\"bra\".*>(.*)</span>");
}

static RadioStation parse_stnblock(const QString& data)
{
	RadioStation station;

	QString stn1block = extract_regexp(data, "<div class=\"stn1\".*>(.*)<div");

	parse_stn1_block(stn1block, station);
	station.index = extract_regexp(data, "tf\\(([0-9]+),this").toInt();

	return station;
}

static QList<Stream> extract_streams(const QString& data)
{
	QList<Stream> streams;
	QRegExp re("\\[(\\'.+)\\]");
	re.setMinimal(true);

	int offset = 0;
	int index = re.indexIn(data, offset);
	while(index >= 0)
	{
		QString cap = re.cap(1);
		QStringList items = cap.split(",");
		if(items.size() < 8){
			offset += index + 1;
			index = re.indexIn(data, offset);
			continue;
		}

		for(QString& item : items)
		{
			if(item.startsWith("'")){
				item.remove(0, 1);
			}

			if(item.endsWith("'")){
				item.remove(item.size()-1, 1);
			}
		}

		Stream stream;
		stream.url = items[0].trimmed();
		stream.url.replace("\\/", "/");
		stream.type = items[1].trimmed();
		stream.bitrate = items[2].trimmed();
		stream.index = items[7].trimmed().toInt();

		if(!stream.type.isEmpty()){
			streams << stream;
		}

		offset = index + 1;

		index = re.indexIn(data, offset);
	}

	return streams;
}

#include "Utils/Utils.h"
FMStreamParser::FMStreamParser(const QByteArray& data)
{
	m = Pimpl::make<Private>();

	QList<RadioStation> stations;
	QString text(data);
	const QList<Stream> streams = extract_streams(data);

	int offset = 0;

	QRegExp re("stnblock.*>(.*)");

	int index = re.indexIn(text);
	while(index >= 0)
	{
		stations << parse_stnblock(re.cap(1));

		offset = index + 1;
		index = re.indexIn(text, offset);
	}

	Util::sort(stations, [](const RadioStation& s1, const RadioStation& s2){
		return (s1.name < s2.name);
	});

	for(const Stream& stream : streams)
	{
		for(RadioStation& station : stations)
		{
			if(station.index == stream.index)
			{
				station.streams << stream;
			}
		}
	}

	for(RadioStation& station : stations)
	{
		if(station.streams.count() > 0)
		{
			m->stations << station;
		}
	}
}

FMStreamParser::~FMStreamParser() {}

QList<RadioStation> FMStreamParser::stations() const
{
	return m->stations;
}
