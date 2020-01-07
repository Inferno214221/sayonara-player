/* Converter.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



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

	Private(int quality) :
		current_index(0),
		num_commands(0),
		num_errors(0),
		num_processes(0),
		quality(quality),
		stopped(false)
	{}
};

Converter::Converter(int quality, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(quality);
}

Converter::~Converter()
{
	Util::File::delete_files(m->log_files);
}

QString Converter::log_directory() const
{
	return Util::File::clean_filename(Util::sayonara_path("encoder-logs"));
}

QString Converter::target_directory() const
{
	return m->target_dir;
}

void Converter::add_metadata(const MetaDataList& v_md)
{
	m->v_md.clear();

	QStringList formats = supported_input_formats();
	for(const MetaData& md : v_md)
	{
		QString filepath = md.filepath();
		for(const QString& format : formats)
		{
			if(filepath.endsWith(format, Qt::CaseInsensitive)){
				m->v_md << md;
				break;
			}
		}
	}
}

void Converter::start(int num_threads, const QString& target_directory)
{
	m->num_processes = num_threads;
	m->target_dir = target_directory;
	m->running_processes.clear();
	m->num_errors = 0;
	m->processes.clear();
	m->current_index = 0;
	m->stopped = false;

	for(const MetaData& md : m->v_md)
	{
		m->processes << process_entry(md);
	}

	m->num_commands = m->v_md.count();

	for(int i=0; i<std::min(m->processes.size(), m->num_processes); i++)
	{
		QString process_name = binary();
		QStringList arguments = m->processes.takeFirst();

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

bool Converter::is_available() const
{
	return QProcess::startDetached(binary(), {"--version"});
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

	Util::File::create_dir(log_directory());
	QString log_file = log_directory() + "/" + QString("encoder_%1_%2.out")
															.arg(binary())
															.arg(m->current_index);

	m->log_files << log_file;

	int id = Util::random_number(100, 1000000);

	auto* process = new QProcess(this);
	process->setStandardOutputFile(log_file);
	process->setStandardErrorFile(log_file);
	process->setProperty("id", id);
	m->running_processes.insert(id, process);

	connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &Converter::process_finished);
	connect(process, &QProcess::errorOccurred, this, &Converter::error_occured);

	sp_log(Log::Debug, this) << "Starting: " << command << " " << arguments.join(" ");
	process->start(command, arguments);

	return true;
}

void Converter::process_finished(int ret, QProcess::ExitStatus exit_status)
{
	Q_UNUSED(exit_status)

	auto* process = static_cast<QProcess*>(sender());

	sp_log(Log::Debug, this) << "process finished";
	if(ret != 0){
		m->num_errors++;
		sp_log(Log::Warning, this) << "Encoding process failed with code " << ret << process->program();
	}

	int id = process->property("id").toInt();
	m->running_processes.remove(id);

	emit sig_progress( 100 - (m->processes.size() * 100) / m->num_commands );

	if(!m->processes.isEmpty() && !m->stopped)
	{
		QStringList arguments = m->processes.takeFirst();
		start_process(binary(), arguments);
	}

	else if(m->running_processes.isEmpty()){
		emit sig_finished();
	}

	else {
		sp_log(Log::Warning, this) << "Something strange happened";
	}
}

void Converter::error_occured(QProcess::ProcessError err)
{
	auto* p = static_cast<QProcess*>(this->sender());

	sp_log(Log::Warning, this) << p->program() << ": " << p->arguments().join(", ");
	sp_log(Log::Warning, this) << "Error: QProcess:ProcessError " << p->errorString();

	process_finished(10000 + err, QProcess::ExitStatus::NormalExit);
}
