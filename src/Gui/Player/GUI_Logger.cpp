/* GUI_Logger.cpp */

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

#include "GUI_Logger.h"
#include "Gui/Player/ui_GUI_Logger.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Logger/LoggerUtils.h"
#include "Utils/Message/Message.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QStringList>
#include <QTextEdit>
#include <utility>

Q_GLOBAL_STATIC(LogObject, logObject) // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

namespace
{
	struct LogLine
	{
		QDateTime dateTime;
		Log logType;
		QString className;
		QString logData;
	};

	std::pair<QString, QString> formattingInfo(const Log logType)
	{
		using FormattingInfo = std::pair<QString, QString>;
		const auto map = QMap<Log, FormattingInfo> {
			{Log::Info,    std::make_pair("#00AA00", "Info")},
			{Log::Warning, std::make_pair("#EE0000", "Warning")},
			{Log::Debug,   std::make_pair("#7A7A00", "Debug")},
			{Log::Develop, std::make_pair("#6A6A00", "Develop")},
			{Log::Crazy,   std::make_pair("#5A5A00", "Verbose")}};

		return map[logType];
	}

	bool isLogLevelSufficientForType(const int logLevel, const Log logType)
	{
		return ((logLevel < 1) && (logType == Log::Debug)) ||
		       ((logLevel < 2) && (logType == Log::Develop)) ||
		       ((logLevel < 3) && (logType == Log::Develop));
	}

	QString formatLogLine(const LogLine& logLine)
	{
		if(logLine.logData.trimmed().isEmpty())
		{
			return {};
		}

		const auto [htmlColor, typeStr] = formattingInfo(logLine.logType);
		const auto className = logLine.className.isEmpty()
		                       ? "::"
		                       : logLine.className;

		constexpr const auto* LogLineTemplate = R"(
<table style="font-family: Monospace;"><tr>
<td>[%1]</td>
<td><div style="color: %2;">%3:</div></td>
<td><div style="color: #0000FF;">%4</div>:</td>
<td>%5</td>
</tr></table>)";

		return QString(LogLineTemplate)
			.arg(logLine.dateTime.toString("hh:mm:ss.zzz"))
			.arg(htmlColor).arg(typeStr)
			.arg(className)
			.arg(logLine.logData);
	}

	void writeLogLineToTextEdit(const LogLine& logLine, QTextEdit* textEdit, const QString& module = QString())
	{
		if(module.isEmpty() || (module == logLine.className))
		{
			const auto formattedString = formatLogLine(logLine);
			if(!formattedString.isEmpty())
			{
				textEdit->append(formattedString);
			}
		}
	}
}

[[maybe_unused]] LogObject::LogObject(QObject* parent) :
	QObject(parent),
	LogListener() {}

LogObject::~LogObject() = default;

void LogObject::addLogLine(const LogEntry& le)
{
	emit sigNewLog(QDateTime::currentDateTime(), le.type, le.className, le.message);
}

struct GUI_Logger::Private
{
	QList<LogLine> logLines;
	Util::Set<QString> modules;

	Private()
	{
		modules << "";
	}
};

GUI_Logger::GUI_Logger(QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>();
	connect(logObject(), &LogObject::sigNewLog, this, &GUI_Logger::logReady, Qt::QueuedConnection);

	Logger::registerLogListener(GUI_Logger::logListener());
}

GUI_Logger::~GUI_Logger() = default;

void GUI_Logger::initUi()
{
	if(ui)
	{
		return;
	}

	ui = std::make_shared<Ui::GUI_Logger>();
	ui->setupUi(this);

	ui->comboLogLevel->addItem(Lang::get(Lang::Default));
	ui->comboLogLevel->addItem("Debug");
	ui->comboLogLevel->addItem("Develop");
	ui->comboLogLevel->addItem("Crazy");

	for(const auto& logLine: m->logLines)
	{
		writeLogLineToTextEdit(logLine, ui->teLog);
	}

	for(const auto& module: m->modules)
	{
		ui->comboModules->addItem(module);
	}

	languageChanged();

	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QWidget::close);
	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_Logger::saveClicked);
	connect(ui->comboModules, &QComboBox::currentTextChanged, this, &GUI_Logger::currentModuleChanged);
	connect(ui->comboLogLevel, combo_current_index_changed_int, this, &GUI_Logger::loglevelChanged);
}

void GUI_Logger::currentModuleChanged(const QString& module)
{
	ui->teLog->clear();

	for(const auto& logLine: m->logLines)
	{
		writeLogLineToTextEdit(logLine, ui->teLog, module);
	}
}

LogListener* GUI_Logger::logListener()
{
	return logObject();
}

void
GUI_Logger::logReady(const QDateTime& dateTime, const Log logType, const QString& className, const QString& logData)
{
	if(!isLogLevelSufficientForType(GetSetting(Set::Logger_Level), logType))
	{
		return;
	}

	const auto logLine = LogLine {dateTime, logType, className, logData};
	m->logLines << logLine;
	m->modules.insert(className);

	if(ui)
	{
		if(ui->comboModules->findText(className) < 0)
		{
			ui->comboModules->addItem(className);
		}

		writeLogLineToTextEdit(logLine, ui->teLog);
	}
}

void GUI_Logger::loglevelChanged(int index)
{
	SetSetting(Set::Logger_Level, index);
	currentModuleChanged(ui->comboModules->currentText());
}

void GUI_Logger::saveClicked()
{
	const auto filename = QFileDialog::getSaveFileName(
		this,
		Lang::get(Lang::SaveAs),
		QDir::homePath(), "*.log");

	if(!filename.isEmpty())
	{
		const auto data = ui->teLog->toPlainText().toLocal8Bit();
		const auto success = Util::File::writeFile(data, filename);
		if(!success)
		{
			Message::warning(tr("Cannot open file") + " " + filename);
		}
	}
}

void GUI_Logger::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);
		ui->labLogLevel->setText(Lang::get(Lang::LogLevel));

		setWindowTitle(Lang::get(Lang::Logger));
	}
}

void GUI_Logger::showEvent(QShowEvent* e)
{
	initUi();

	Dialog::showEvent(e);

	ui->comboLogLevel->setCurrentIndex(GetSetting(Set::Logger_Level));
}
