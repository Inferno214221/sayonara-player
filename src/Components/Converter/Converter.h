/* Converter.h */

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
	Converter(int quality, QObject* parent=nullptr);
	virtual ~Converter();

	virtual QStringList supported_input_formats() const=0;
	virtual QString binary() const=0;

	QString		log_directory() const;
	QString		target_directory() const;
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
	void start(int num_threads, const QString& target_dir);
	void stop();

private slots:
	void process_finished(int ret, QProcess::ExitStatus status);
	void error_occured(QProcess::ProcessError err);
};

#endif // OGGCONVERTER_H
