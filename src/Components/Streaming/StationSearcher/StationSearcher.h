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

private:
	void start_call();

public:

	enum Mode
	{
		NewSearch,
		Incremental,
		Style
	};

	StationSearcher(QObject* parent=nullptr);
	~StationSearcher();

	void search_style(const QString& style);
	void search_station(const QString& name);
	void search_previous();
	void search_next();

	bool can_search_next() const;
	bool can_search_previous() const;
	Mode mode() const;

	QList<RadioStation> found_stations() const;

private slots:
	void search_finished();
};

#endif // STATIONSEARCHER_H
