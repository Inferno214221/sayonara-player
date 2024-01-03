/* GUI_Logger.h */

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

#ifndef GUI_LOGGER_H
#define GUI_LOGGER_H

#include "Utils/Logger/LogListener.h"
#include "Gui/Utils/Widgets/Dialog.h"
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
		void sigNewLog(const QDateTime& t, Log logType, const QString& className, const QString& str);

	public:
		[[maybe_unused]] explicit LogObject(QObject* parent = nullptr);
		~LogObject() override;

		void addLogLine(const LogEntry& le) override;
};

class GUI_Logger :
	public Gui::Dialog
{
	Q_OBJECT
	UI_CLASS_SHARED_PTR(GUI_Logger)
	PIMPL(GUI_Logger)

	public:
		explicit GUI_Logger(QWidget* parent = nullptr);
		~GUI_Logger() override;

		static LogListener* logListener();

	private:
		void initUi();

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void currentModuleChanged(const QString& module);
		void logReady(const QDateTime& dateTime, Log log_type, const QString& class_name, const QString& str);
		void loglevelChanged(int index);
		void saveClicked();

	protected:
		void showEvent(QShowEvent* e) override;
		void languageChanged() override;
};

#endif // GUI_LOGGER_H
