#ifndef OGG_CONVERTER_H
#define OGG_CONVERTER_H

#include "Converter.h"

class MetaDataList;
class MetaData;

class OggConverter :
		public Converter
{
	Q_OBJECT

public:
	OggConverter(int quality, QObject* parent=nullptr);
	~OggConverter() override;

	QStringList supported_input_formats() const override;

protected:
	QString binary() const override;
	QStringList process_entry(const MetaData& md) const override;
	QString extension() const override;
};

#endif // OGGCONVERTER_H
