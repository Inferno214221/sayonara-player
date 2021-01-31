/* Converter.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara Indexyer
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
	void sigFinished();
	void sigProgress(int percent);

public:
	Converter(int quality, QObject* parent=nullptr);
	virtual ~Converter();

	virtual QStringList supportedInputFormats() const=0;
	virtual QString binary() const=0;

	QString		loggingDirectory() const;
	QString		targetDirectory() const;
	QString		targetFile(const MetaData& md) const;
	void 		addMetadata(const MetaDataList& tracks);
	int 		errorCount() const;
	int 		quality() const;
	int         initialCount() const;
	int			fileCount() const;
	bool		isAvailable() const;

private:
	bool startProcess(const QString& processName, const QStringList& arguments);

protected:
	virtual QStringList processEntry(const MetaData& md) const=0;
	virtual QString extension() const=0;

public slots:
	void start(int numThreads, const QString& targetDir);
	void stop();

private slots:
	void processFinished(int ret, QProcess::ExitStatus status);
	void errorOccured(QProcess::ProcessError err);
};

#endif // OGGCONVERTER_H
