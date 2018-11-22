#ifndef LAMECONVERTER_H
#define LAMECONVERTER_H

#include "Converter.h"

class LameConverter :
		public Converter
{
	Q_OBJECT
	PIMPL(LameConverter)

	public:
		LameConverter(int num_threads, bool cbr, int quality, QObject* parent);
		~LameConverter() override;

		QStringList supported_input_formats() const override;

		// Converter interface
	protected:
		QString binary() const override;
		QStringList process_entry(const MetaData& md) const override;
		QString extension() const override;
};

#endif // LAMECONVERTER_H
