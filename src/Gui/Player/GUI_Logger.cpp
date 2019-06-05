/* GUI_Logger.cpp */

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

Q_GLOBAL_STATIC(LogObject, log_object)

struct LogLine
{
	QDateTime	date_time;
	Log			log_type;
	QString		class_name;
	QString	str;

	LogLine(const QDateTime& date_time, Log log_type, const QString& class_name, const QString& str) :
		date_time(date_time),
		log_type(log_type),
		class_name(class_name),
		str(str)
	{}

	QString to_string() const
	{
		int log_level = GetSetting(Set::Logger_Level);
		QString log_line = "<table style=\"font-family: Monospace;\">";
		QString html_color, type_str;
		switch(log_type)
		{
			case Log::Info:
				html_color = "#00AA00";
				type_str = "Info";
				break;
			case Log::Warning:
				html_color = "#EE0000";
				type_str = "Warning";
				break;
			case Log::Error:
				html_color = "#EE0000";
				type_str = "Error";
				break;
			case Log::Debug:
				html_color = "#7A7A00";
				type_str = "Debug";
				if(log_level < 1) {
					return QString();
				}
				break;

			case Log::Develop:
				html_color = "#6A6A00";
				type_str = "Dev";
				if(log_level < 2){
					return QString();
				}
				break;

			case Log::Crazy:
				html_color = "#5A5A00";
				type_str = "CrazyLog";
				if(log_level < 3){
					return QString();
				}
				break;
			default:
				type_str = "Debug";
				break;
		}

		log_line += "<tr>";
		log_line += "<td>[" + date_time.toString("hh:mm:ss") + "." + QString::number(date_time.time().msec()) + "]</td>";
		log_line += "<td><div style=\"color: " + html_color + ";\">" + type_str + ": </div></td>";

		if(!class_name.isEmpty())
		{
			log_line += "<td><div style=\"color: #0000FF;\">" + class_name + "</div>:</td>";
		}

		log_line += "<td>" + str + "</td>";
		log_line += "</tr>";
		log_line += "</table>";

		return log_line;
	}
};

LogObject::LogObject(QObject* parent) :
	QObject(parent),
	LogListener()
{}

LogObject::~LogObject() {}

void LogObject::add_log_line(const LogEntry& le)
{
	emit sig_new_log(QDateTime::currentDateTime(), le.type, le.class_name, le.message);
}

#include <QList>

struct GUI_Logger::Private
{
	QList<LogLine> buffer;
	QStringList modules;

	Private()
	{
		modules << "";
	}
};


GUI_Logger::GUI_Logger(QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>();
	connect(log_object (), &LogObject::sig_new_log, this, &GUI_Logger::log_ready, Qt::QueuedConnection);

	Logger::register_log_listener(this->get_log_listener());
}

GUI_Logger::~GUI_Logger()
{
	if(ui){
		delete ui; ui=nullptr;
	}
}


void GUI_Logger::init_ui()
{
	if(ui) {
		return;
	}

	ui = new Ui::GUI_Logger;
	ui->setupUi(this);

	for(const LogLine& line : Algorithm::AsConst(m->buffer))
	{
		ui->te_log->append(line.to_string());
	}

	for(const QString& module : m->modules){
		ui->combo_modules->addItem(module);
	}

	language_changed();

	connect(ui->btn_close, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_Logger::save_clicked);
	connect(ui->combo_modules, &QComboBox::currentTextChanged, this, &GUI_Logger::current_module_changed);
}

QString GUI_Logger::calc_log_line(const LogLine& log_line)
{
	m->buffer << log_line;

	if(!m->modules.contains(log_line.class_name))
	{
		int i=0;
		for(; i<m->modules.size(); i++)
		{
			if(log_line.class_name < m->modules[i]){

				break;
			}
		}

		m->modules.insert(i, log_line.class_name);

		if(ui)
		{
			ui->combo_modules->insertItem(i, log_line.class_name);
		}
	}

	return log_line.to_string();
}

void GUI_Logger::current_module_changed(const QString& module)
{
	ui->te_log->clear();

	for(const LogLine& log_line : m->buffer)
	{
		if((log_line.class_name == module) || module.isEmpty())
		{
			ui->te_log->append(log_line.to_string());
		}
	}
}

void GUI_Logger::language_changed()
{
	if(ui)
	{
		ui->retranslateUi(this);
		ui->btn_close->setText(Lang::get(Lang::Close));
		ui->btn_save->setText(Lang::get(Lang::SaveAs).triplePt());
		this->setWindowTitle(Lang::get(Lang::Logger));
	}
}


LogListener* GUI_Logger::get_log_listener()
{
	return log_object ();
}


void GUI_Logger::log_ready(const QDateTime& t, Log log_type, const QString& class_name, const QString& message)
{
	LogLine log_line(t, log_type, class_name, message);
	QString str = calc_log_line(log_line);

	if(ui)
	{
		ui->te_log->append(str);
	}
}

void GUI_Logger::save_clicked()
{
	QString filename = QFileDialog::getSaveFileName(
						   this,
						   Lang::get(Lang::SaveAs),
						   QDir::homePath(), "*.log");

	if(filename.isEmpty()){
		return;
	}

	QFile f(filename);
	bool is_open = f.open(QFile::WriteOnly);
	if(is_open){
		f.write(ui->te_log->toPlainText().toUtf8());
		f.close();
	}

	else {
		Message::warning(tr("Cannot open file") + " " + filename);
	}
}


void GUI_Logger::showEvent(QShowEvent* e)
{
	init_ui();

	Widget::showEvent(e);
}

