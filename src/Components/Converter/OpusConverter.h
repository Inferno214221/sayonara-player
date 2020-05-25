#ifndef OPUSCONVERTER_H
#define OPUSCONVERTER_H

#include "Converter.h"

class OpusConverter :
	public Converter
{
	Q_OBJECT
	PIMPL(OpusConverter)

	public:
		OpusConverter(bool cbr, int quality, QObject* parent);
		~OpusConverter() override;

		QStringList supportedInputFormats() const override;

		// Converter interface
	protected:
		QString binary() const override;
		QStringList processEntry(const MetaData& md) const override;
		QString extension() const override;
};

#endif // OPUSCONVERTER_H
