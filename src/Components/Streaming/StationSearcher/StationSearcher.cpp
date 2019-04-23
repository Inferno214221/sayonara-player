#include "StationSearcher.h"
#include "FMStreamParser.h"
#include "Utils/WebAccess/AsyncWebAccess.h"

#include <QUrl>

struct StationSearcher::Private
{
	QList<RadioStation> found_stations;

	QString get_url(const QString& search_string)
	{
		return QString("http://fmstream.org/index.php?s=%1&cm=0").arg(search_string);
	}
};

StationSearcher::StationSearcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

StationSearcher::~StationSearcher() {}

void StationSearcher::search_station(const QString& name)
{
	QString url = m->get_url(name);
	AsyncWebAccess* wa = new AsyncWebAccess(this);

	connect(wa, &AsyncWebAccess::sig_finished, this, &StationSearcher::search_finished);

	wa->run(url);
}

QList<RadioStation> StationSearcher::found_stations() const
{
	return m->found_stations;
}

void StationSearcher::search_finished()
{
	AsyncWebAccess* wa = static_cast<AsyncWebAccess*>(sender());

	FMStreamParser parser(wa->data());
	m->found_stations = parser.stations();

	wa->deleteLater();

	emit sig_stations_found();
}
