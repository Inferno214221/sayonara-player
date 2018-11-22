#ifndef LAMECONVERTER_H
#define LAMECONVERTER_H

#include "Converter.h"

class LameConverter :
		public Converter
{
	Q_OBJECT
	PIMPL(LameConverter)

public:
	LameConverter(const QString& target_dir, int num_threads, bool cbr, int quality, QObject* parent);
	~LameConverter() override;

	// Converter interface
protected:
	QStringList get_process_entry(const MetaData& md) const override;
	QString extension() const override;
};

#endif // LAMECONVERTER_H
