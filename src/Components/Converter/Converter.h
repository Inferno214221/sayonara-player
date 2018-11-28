#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H

#include <QObject>
#include <QProcess>
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
	Converter(int num_threads, int quality, QObject* parent=nullptr);
	virtual ~Converter();

	virtual QStringList supported_input_formats() const=0;
	virtual QString binary() const=0;

	QString		log_directory() const;
	QString		target_file(const MetaData& md) const;
	void 		add_metadata(const MetaDataList& v_md);
	int 		num_errors() const;
	int 		quality() const;
	int			num_files() const;
	bool		is_available() const;

private:
	bool start_process(const QString& process_name, const QStringList& arguments);

protected:
	virtual QStringList process_entry(const MetaData& md) const=0;
	virtual QString extension() const=0;

public slots:
	void start(const QString& target_dir);
	void stop();

private slots:
	void process_finished(int ret);
	void error_occured(QProcess::ProcessError err);
};

#endif // OGGCONVERTER_H
