/* GUI_Logger.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#ifndef GUI_LOGGER_H
#define GUI_LOGGER_H

#include "Utils/Logger/LogListener.h"
#include "GUI/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

#include <QStringList>
#include <QWidget>

class QShowEvent;

UI_FWD(GUI_Logger)

class LogObject :
		public QObject,
		public LogListener
{
		Q_OBJECT

	signals:
		void sig_new_log(const QDateTime& t, Log log_type, const QString& class_name, const QString& str);

	public:
		explicit LogObject(QObject* parent=nullptr);
		~LogObject();

		void add_log_line(const LogEntry& le) override;
};

struct LogLine;
class GUI_Logger :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_Logger)
	PIMPL(GUI_Logger)

	public:
		explicit GUI_Logger(QWidget *parent = 0);
		~GUI_Logger();

		LogListener* get_log_listener();

	protected:
		void showEvent(QShowEvent* e) override;
		void language_changed() override;

		void init_ui();
		QString calc_log_line(const LogLine& log_line);

	private slots:
		void current_module_changed(const QString& module);
		void log_ready(const QDateTime& t, Log log_type, const QString& class_name, const QString& str);
		void save_clicked();
};

#endif // GUI_LOGGER_H
