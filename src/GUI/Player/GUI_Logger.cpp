/* GUI_Logger.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "GUI/Player/ui_GUI_Logger.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Logger/LoggerUtils.h"
#include "Utils/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"

#include <QStringList>
#include <QTextEdit>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QDateTime>

Q_GLOBAL_STATIC(LogObject, log_object)

LogObject::LogObject(QObject* parent) :
	QObject(parent),
	LogListener()
{}

LogObject::~LogObject() {}

void LogObject::add_log_line(const LogEntry& le)
{
	emit sig_new_log(QDateTime::currentDateTime(), le.type, le.class_name, le.message);
}


GUI_Logger::GUI_Logger(QWidget *parent) :
	Widget(parent)
{
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

	for(const QString& line : Util::AsConst(_buffer))
	{
		ui->te_log->append(line);
	}

	_buffer.clear();
	language_changed();

	connect(ui->btn_close, &QPushButton::clicked, this, &QWidget::close);
	connect(ui->btn_save, &QPushButton::clicked, this, &GUI_Logger::save_clicked);
}

QString GUI_Logger::calc_log_line(const QDateTime &t, Log log_type, const QString& class_name, const QString& str)
{
	int log_level = _settings->get<Set::Logger_Level>();
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
	log_line += "<td>[" + t.toString("hh:mm:ss") + "." + QString::number(t.time().msec()) + "]</td>";
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


void GUI_Logger::log_ready(const QDateTime& t, Log log_type, const QString& class_name, const QString& str)
{
	if(!ui){
		_buffer << calc_log_line(t, log_type, class_name, str);
	}

	else {
		ui->te_log->append(calc_log_line(t, log_type, class_name, str));
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

