/* Converter.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QVariant>

using ProcessMap = QList<QStringList>;

struct Converter::Private
{
	QStringList logFiles;
	ProcessMap processes;
	MetaDataList tracks;
	QString targetDirectory;
	QMap<int, QProcess*> runningProcesses;

	int initialCount;
	int currentIndex;
	int commandCount;
	int errorCount;
	int processCount;
	int quality;
	bool stopped;

	Private(int quality) :
		initialCount(0),
		currentIndex(0),
		commandCount(0),
		errorCount(0),
		processCount(0),
		quality(quality),
		stopped(false) {}
};

Converter::Converter(int quality, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(quality);

}

Converter::~Converter()
{
	Util::File::deleteFiles(m->logFiles);
}

QString Converter::loggingDirectory() const
{
	return Util::tempPath("encoder-logs");
}

QString Converter::targetDirectory() const
{
	return m->targetDirectory;
}

void Converter::addMetadata(const MetaDataList& tracks)
{
	m->initialCount = tracks.count();
	m->tracks.clear();

	QStringList formats = supportedInputFormats();
	for(const MetaData& md: tracks)
	{
		const QString filepath(md.filepath());
		for(const QString& format: formats)
		{
			if(filepath.endsWith(format, Qt::CaseInsensitive))
			{
				m->tracks << md;
				break;
			}
		}
	}
}

void Converter::start(int numThreads, const QString& targetDirectoryectory)
{
	m->processCount = numThreads;
	m->targetDirectory = targetDirectoryectory;
	m->runningProcesses.clear();
	m->errorCount = 0;
	m->processes.clear();
	m->currentIndex = 0;
	m->stopped = false;

	for(const MetaData& md: m->tracks)
	{
		m->processes << processEntry(md);
	}

	m->commandCount = m->tracks.count();

	auto processCount = std::min(m->processes.size(), m->processCount);
	for(int i = 0; i < processCount; i++)
	{
		QString process_name = binary();
		QStringList arguments = m->processes.takeFirst();

		startProcess(process_name, arguments);
	}
}

void Converter::stop()
{
	m->stopped = true;

	const auto processKeys = m->runningProcesses.keys();
	for(int key: processKeys)
	{
		QProcess* p = m->runningProcesses.value(key);
		if(p)
		{
			p->kill();
		}
	}
}

int Converter::errorCount() const
{
	return m->errorCount;
}

int Converter::quality() const
{
	return m->quality;
}

int Converter::fileCount() const
{
	return m->tracks.count();
}

int Converter::initialCount() const
{
	return m->initialCount;
}

bool Converter::isAvailable() const
{
	return QProcess::startDetached(binary(), {"--version"});
}

QString Converter::targetFile(const MetaData& md) const
{
	const auto [dirname, filename] = Util::File::splitFilename(md.filepath());

	QString target = Util::File::cleanFilename(m->targetDirectory + "/" + filename);
	target = target.left(target.lastIndexOf(".")) + "." + extension();

	return target;
}

bool Converter::startProcess(const QString& command, const QStringList& arguments)
{
	m->currentIndex++;

	Util::File::createDir(loggingDirectory());
	const auto logFile = QString("%1/encoder_%2_%3.out")
		.arg(loggingDirectory())
		.arg(binary())
		.arg(m->currentIndex);

	m->logFiles << logFile;

	int id = Util::randomNumber(100, 1000000);

	auto* process = new QProcess(this);
	process->setStandardOutputFile(logFile);
	process->setStandardErrorFile(logFile);
	process->setProperty("id", id);
	m->runningProcesses.insert(id, process);

	connect(process,
	        static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
	        this,
	        &Converter::processFinished);
	connect(process, &QProcess::errorOccurred, this, &Converter::errorOccured);

	spLog(Log::Debug, this) << "Starting: " << command << " " << arguments.join(" ");
	process->start(command, arguments);

	return true;
}

void Converter::processFinished(int ret, QProcess::ExitStatus exit_status)
{
	Q_UNUSED(exit_status)

	auto* process = static_cast<QProcess*>(sender());

	spLog(Log::Debug, this) << "process finished";
	if(ret != 0)
	{
		m->errorCount++;
		spLog(Log::Warning, this) << "Encoding process failed with code " << ret << process->program();
	}

	int id = process->property("id").toInt();
	m->runningProcesses.remove(id);

	emit sigProgress(100 - (m->processes.size() * 100) / m->commandCount);

	if(!m->processes.isEmpty() && !m->stopped)
	{
		QStringList arguments = m->processes.takeFirst();
		startProcess(binary(), arguments);
	}

	else if(m->runningProcesses.isEmpty())
	{
		emit sigFinished();
	}

	else
	{
		spLog(Log::Warning, this) << "Something strange happened";
	}
}

void Converter::errorOccured(QProcess::ProcessError err)
{
	auto* p = static_cast<QProcess*>(this->sender());

	spLog(Log::Warning, this) << p->program() << ": " << p->arguments().join(", ");
	spLog(Log::Warning, this) << "Error: QProcess:ProcessError " << p->errorString();

	processFinished(10000 + err, QProcess::ExitStatus::NormalExit);
}
