#include "Converter.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVariant>


using ProcessMap=QList<QStringList>;

struct Converter::Private
{
	QStringList log_files;
	ProcessMap processes;
	MetaDataList v_md;
	QString target_dir;
	QMap<int, QProcess*> running_processes;

	int current_index;
	int num_commands;
	int num_errors;
	int num_processes;
	int quality;
	bool stopped;

	Private(const QString& target_dir, int num_processes, int quality) :
		target_dir(target_dir),
		current_index(0),
		num_errors(0),
		num_processes(num_processes),
		quality(quality),
		stopped(false)
	{}
};

Converter::Converter(const QString& target_dir, int num_processes, int quality, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(target_dir, num_processes, quality);
}

Converter::~Converter()
{
	Util::File::delete_files(m->log_files);
}

void Converter::add_metadata(const MetaDataList& v_md)
{
	m->v_md.clear();

	for(const MetaData& md : v_md)
	{
		QString f = md.filepath();
		if(f.endsWith("flac", Qt::CaseInsensitive) ||
			f.endsWith("wav", Qt::CaseInsensitive))
		{
			m->v_md << md;
		}
	}
}

void Converter::start()
{
	m->running_processes.clear();
	m->num_errors = 0;
	m->processes.clear();
	m->current_index = 0;
	m->stopped = false;

	for(const MetaData& md : m->v_md)
	{
		m->processes << get_process_entry(md);
	}

	m->num_commands = m->v_md.count();

	for(int i=0; i<std::min(m->processes.size(), m->num_processes); i++)
	{
		QStringList process = m->processes.takeFirst();
		QString process_name = process.takeFirst();
		QStringList arguments = process;
		sp_log(Log::Debug, this) << process_name << " " << arguments.join(" ");
		start_process(process_name, arguments);
	}
}

void Converter::stop()
{
	m->stopped = true;

	for(int key : m->running_processes.keys())
	{
		QProcess* p = m->running_processes.value(key);
		if(!p){
			continue;
		}

		p->kill();
	}
}

int Converter::num_errors() const
{
	return m->num_errors;
}

int Converter::quality() const
{
	return m->quality;
}

int Converter::num_files() const
{
	return m->v_md.count();
}

QString Converter::target_file(const MetaData& md) const
{
	QString filename, dirname;
	Util::File::split_filename(md.filepath(), dirname, filename);
	QString target = Util::File::clean_filename(m->target_dir + "/" + filename);
	target = target.left(target.lastIndexOf(".")) + "." + extension();

	return target;
}

bool Converter::start_process(const QString& command, const QStringList& arguments)
{
	m->current_index++;

	QString log_file = QString("encoder_%1.out").arg(m->current_index);
	m->log_files << log_file;

	int id = Util::random_number(100, 1000000);

	QProcess* process = new QProcess(this);
	process->setStandardOutputFile(Util::sayonara_path(QString("encoder_%1.out").arg(m->current_index)));
	process->setStandardErrorFile(Util::sayonara_path(QString("encoder_%1.out").arg(m->current_index)));
	process->setProperty("id", id);
	m->running_processes.insert(id, process);

	connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, &Converter::process_finished);

	process->start(command, arguments);

	return true;
}

void Converter::process_finished(int ret)
{
	QProcess* process = static_cast<QProcess*>(sender());
	Q_UNUSED(process);

	if(ret != 0){
		m->num_errors++;
		sp_log(Log::Warning, this) << "Encoding process failed with code " << ret << process->program();
	}

	int id = process->property("id").toInt();
	m->running_processes.remove(id);

	emit sig_progress( 100 - (m->processes.size() * 100) / m->num_commands );

	if(!m->processes.isEmpty() && !m->stopped)
	{
		QStringList process = m->processes.takeFirst();
		QString process_name = process.takeFirst();
		QStringList arguments = process;
		start_process(process_name, arguments);
	}

	else if(m->running_processes.size() == 0){
		emit sig_finished();
	}
}
