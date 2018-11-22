#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>
#include "Utils/Pimpl.h"

class MetaDataList;
class MetaData;

class Converter :
		public QObject
{
	Q_OBJECT
	PIMPL(Converter)

signals:
	void sig_finished();
	void sig_progress(int percent);

public:
	Converter(const QString& target_dir, int num_threads, int quality, QObject* parent=nullptr);
	virtual ~Converter();

	QString		target_file(const MetaData& md) const;
	void 		add_metadata(const MetaDataList& v_md);
	int 		num_errors() const;
	int 		quality() const;
	int			num_files() const;

private:
	bool start_process(const QString& process_name, const QStringList& arguments);

protected:
	virtual QStringList get_process_entry(const MetaData& md) const=0;
	virtual QString extension() const=0;

public slots:
	void start();
	void stop();

private slots:
	void process_finished(int ret);
};

#endif // OGGCONVERTER_H
