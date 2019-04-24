#ifndef FMSTREAMPARSER_H
#define FMSTREAMPARSER_H

#include "RadioStation.h"
#include "Utils/Pimpl.h"

#include <QList>
#include <array>



class QByteArray;

class FMStreamParser
{
	PIMPL(FMStreamParser)

public:

	enum Encoding
	{
		Utf8=1,
		Latin1=2,
		Local8Bit=3
	};

	using EncodingTuple=std::array<Encoding, 4>;

	FMStreamParser(const QByteArray& data);
	FMStreamParser(const QByteArray& data, EncodingTuple encodings, int index);
	~FMStreamParser();

	QList<RadioStation> stations() const;
};

#endif // FMSTREAMPARSER_H
