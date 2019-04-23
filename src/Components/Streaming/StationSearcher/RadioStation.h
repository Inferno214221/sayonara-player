#ifndef RADIOSTATION_H
#define RADIOSTATION_H

#include <QString>
#include <QList>

struct Stream
{
	int index;
	QString url;
	QString type;
	QString bitrate;
};

struct RadioStation
{
	int index;
	QString name;
	QString location;
	QString style;
	QString frequency;
	QString short_description;
	QString description;
	QString image;

	QList<Stream> streams;
};

#endif // RADIOSTATION_H
