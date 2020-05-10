/* GUI_Logger.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "GUI_Logger.h"
#include "Gui/Player/ui_GUI_Logger.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Logger/LoggerUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDateTime>

namespace Algorithm=Util::Algorithm;

Q_GLOBAL_STATIC(LogObject, logObject)

struct LogLine
{
	QDateTime	dateTime;
	Log			logType;
	QString		className;
	QString		str;

	LogLine(const QDateTime& dateTime, Log logType, const QString& className, const QString& str) :
		dateTime(dateTime),
		logType(logType),
		className(className),
		str(str)
	{}

	QString toString() const
	{
		int logLevel = GetSetting(Set::Logger_Level);
		QString logLine = "<table style=\"font-family: Monospace;\">";
		QString htmlColor, typeStr;
		switch(logType)
		{
			case Log::Info:
				htmlColor = "#00AA00";
				typeStr = "Info";
				break;
			case Log::Warning:
				htmlColor = "#EE0000";
				typeStr = "Warning";
				break;
			case Log::Error:
				htmlColor = "#EE0000";
				typeStr = "Error";
				break;
			case Log::Debug:
				htmlColor = "#7A7A00";
				typeStr = "Debug";
				if(logLevel < 1) {
					return QString();
				}
				break;
			case Log::Develop:
				htmlColor = "#6A6A00";
				typeStr = "Dev";
				if(logLevel < 2){
					return QString();
				}
				break;
			case Log::Crazy:
				htmlColor = "#5A5A00";
				typeStr = "CrazyLog";
				if(logLevel < 3){
					return QString();
				}
				break;
			default:
				typeStr = "Debug";
				break;
		}

		logLine += "<tr>";
		logLine += "<td>[" + dateTime.toString("hh:mm:ss") + "." + QString::number(dateTime.time().msec()) + "]</td>";
		logLine += "<td><div style=\"color: " + htmlColor + ";\">" + typeStr + ": </div></td>";

		if(!className.isEmpty())
		{
			logLine += "<td><div style=\"color: #0000FF;\">" + className + "</div>:</td>";
		}

		logLine += "<td>" + str + "</td>";
		logLine += "</tr>";
		logLine += "</table>";

		return logLine;
	}
};

LogObject::LogObject(QObject* parent) :
	QObject(parent),
	LogListener()
{}

LogObject::~LogObject() = default;

void LogObject::addLogLine(const LogEntry& le)
{
	emit sigNewLog(QDateTime::currentDateTime(), le.type, le.className, le.message);
}

struct GUI_Logger::Private
{
	QList<LogLine> buffer;
	QStringList modules;

	Private()
	{
		modules << "";
	}
};

GUI_Logger::GUI_Logger(QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>();
	connect(logObject (), &LogObject::sigNewLog, this, &GUI_Logger::logReady, Qt::QueuedConnection);

	Logger::registerLogListener(this->logListener());
}

GUI_Logger::~GUI_Logger()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}

void GUI_Logger::initUi()
{
	if(ui) {
		return;
	}

	ui = new Ui::GUI_Logger;
	ui->setupUi(this);

	for(const LogLine& line : Algorithm::AsConst(m->buffer))
	{
		ui->teLog->append(line.toString());
	}

	for(const QString& module : m->modules){
		ui->comboModules->addItem(module);
	}

	languageChanged();

	connect(ui->btnClose, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->btnSave, &QPushButton::clicked, this, &GUI_Logger::saveClicked);
	connect(ui->comboModules, &QComboBox::currentTextChanged, this, &GUI_Logger::currentModuleChanged);
}

QString GUI_Logger::calcLogLine(const LogLine& logLine)
{
	m->buffer << logLine;

	if(!m->modules.contains(logLine.className))
	{
		int i=0;
		for(; i<m->modules.size(); i++)
		{
			if(logLine.className < m->modules[i]){

				break;
			}
		}

		m->modules.insert(i, logLine.className);

		if(ui)
		{
			ui->comboModules->insertItem(i, logLine.className);
		}
	}

	return logLine.toString();
}

void GUI_Logger::currentModuleChanged(const QString& module)
{
	ui->teLog->clear();

	for(const LogLine& logLine : m->buffer)
	{
		if((logLine.className == module) || module.isEmpty())
		{
			ui->teLog->append(logLine.toString());
		}
	}
}

void GUI_Logger::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
		ui->btnClose->setText(Lang::get(Lang::Close));
		ui->btnSave->setText(Lang::get(Lang::SaveAs).triplePt());
		this->setWindowTitle(Lang::get(Lang::Logger));
	}
}

LogListener* GUI_Logger::logListener()
{
	return logObject ();
}

void GUI_Logger::logReady(const QDateTime& t, Log logType, const QString& className, const QString& message)
{
	const LogLine logLine(t, logType, className, message);
	const QString str = calcLogLine(logLine);

	if(ui)
	{
		ui->teLog->append(str);
	}
}

void GUI_Logger::saveClicked()
{
	const QString filename = QFileDialog::getSaveFileName
	(
	   this,
	   Lang::get(Lang::SaveAs),
	   QDir::homePath(), "*.log"
	);

	if(filename.isEmpty()){
		return;
	}

	QFile f(filename);
	bool isOpen = f.open(QFile::WriteOnly);
	if(isOpen){
		f.write(ui->teLog->toPlainText().toUtf8());
		f.close();
	}

	else {
		Message::warning(tr("Cannot open file") + " " + filename);
	}
}

void GUI_Logger::showEvent(QShowEvent* e)
{
	initUi();
	Dialog::showEvent(e);
}

