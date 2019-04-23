#ifndef STATIONSEARCHER_H
#define STATIONSEARCHER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "RadioStation.h"

class StationSearcher : public QObject
{
	Q_OBJECT
	PIMPL(StationSearcher)

signals:
	void sig_stations_found();

public:
	StationSearcher(QObject* parent=nullptr);
	~StationSearcher();

	void search_station(const QString& name);
	QList<RadioStation> found_stations() const;

private slots:
	void search_finished();
};

#endif // STATIONSEARCHER_H
