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
	OggConverter(const QString& target_dir, int num_threads, int quality, QObject* parent=nullptr);
	~OggConverter() override;

protected:
	QStringList get_process_entry(const MetaData& md) const override;
	QString extension() const override;
};

#endif // OGGCONVERTER_H
