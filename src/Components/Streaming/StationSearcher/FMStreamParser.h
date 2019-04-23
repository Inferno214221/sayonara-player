#ifndef FMSTREAMPARSER_H
#define FMSTREAMPARSER_H

#include "RadioStation.h"
#include "Utils/Pimpl.h"
#include <QList>

class QByteArray;

class FMStreamParser
{
	PIMPL(FMStreamParser)

public:
	FMStreamParser(const QByteArray& data);
	~FMStreamParser();

	QList<RadioStation> stations() const;
};

#endif // FMSTREAMPARSER_H
