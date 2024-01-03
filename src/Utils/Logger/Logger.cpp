/* Logger.cpp */

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

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Logger/LogListener.h"

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QPoint>
#include <QDateTime>
#include <QSize>
#include <QMargins>
#include <QRect>
#include <QRegion>

#include "LoggerUtils.h"

#include <ostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef Q_OS_UNIX
#define LOG_RED "\x1B[31m"
#define LOG_GREEN "\x1B[32m"
#define LOG_BLUE "\x1B[34m"
#define LOG_YELLOW "\x1B[33m"
#define LOG_COL_END "\x1B[0m"
#else
#define LOG_RED ""
#define LOG_GREEN ""
#define LOG_BLUE ""
#define LOG_YELLOW ""
#define LOG_COL_END ""
#endif

#ifdef SAYONARA_HAS_CXX_ABI

#include "cxxabi.h"

#endif

using LogListeners = QList<LogListener*>;
Q_GLOBAL_STATIC(LogListeners, log_listeners)

using LogEntries = QList<LogEntry>;
Q_GLOBAL_STATIC(LogEntries, log_buffer)

struct Logger::Private
{
	QString class_name;
	std::stringstream msg;
	Log type;

	Private() {}

	~Private()
	{
		QString type_str;
		std::string color;

		bool ignore = false;

		Settings* s = Settings::instance();
		int logger_level = 0;

		if(s->checkSettings())
		{
			logger_level = GetSetting(Set::Logger_Level);
		}

		switch(type)
		{
			case Log::Info:
				color = LOG_GREEN;
				type_str = "Info";
				break;
			case Log::Warning:
				color = LOG_RED;
				type_str = "Warning";
				break;
			case Log::Error:
				color = LOG_RED;
				type_str = "Error";
				break;
			case Log::Debug:
				color = LOG_YELLOW;
				type_str = "Debug";
				if(logger_level < 1)
				{
					ignore = true;
				}
				break;
			case Log::Develop:
				color = LOG_YELLOW;
				type_str = "Dev";
				if(logger_level < 2)
				{
					ignore = true;
				}
				break;
			case Log::Crazy:
				color = LOG_YELLOW;
				type_str = "CrazyLog";
				if(logger_level < 3)
				{
					ignore = true;
				}

				break;
			case Log::Always:
				[[fallthrough]];
			default:
				color = LOG_YELLOW;
				type_str = "";
				break;
		}

		if(!ignore)
		{
			QString date_time = QDateTime::currentDateTime().toString("hh:mm:ss");

			std::string str(msg.str());
			std::clog
				<< "[" << date_time.toStdString() << "] "
				<< color
				<< type_str.toStdString() << ": "
				<< LOG_COL_END;

			if(!class_name.isEmpty())
			{
				std::clog << LOG_BLUE << class_name.toStdString() << ": " << LOG_COL_END;
			}

			std::clog << str;
			std::clog << std::endl;

			LogEntry le;
			le.className = class_name;
			le.message = QString::fromStdString(str);
			le.type = type;

			log_buffer->push_back(le);

			for(auto it = log_listeners->begin(); it != log_listeners->end(); it++)
			{
				LogListener* log_listener = *it;
				if(log_listener)
				{
					log_listener->addLogLine(le);
				}
			}
		}

		msg.clear();
	}
};

Logger::Logger(const Log& type, const QString& class_name)
{
	m = new Logger::Private();

	m->type = type;
	m->class_name = class_name;
}

Logger::~Logger()
{
	delete m;
	m = nullptr;
}

//static
void Logger::registerLogListener(LogListener* log_listener)
{
	for(auto it = log_buffer->begin(); it != log_buffer->end(); it++)
	{
		log_listener->addLogLine(*it);
	}

	log_listeners->push_back(log_listener);
}

Logger& Logger::operator<<(const QString& msg)
{
	(*this) << msg.toLocal8Bit().constData();
	return *this;
}

Logger& Logger::operator<<(const QStringList& lst)
{
	(*this) << lst.join(",");

	return *this;
}

Logger& Logger::operator<<(const QChar& c)
{
	(*this) << c.toLatin1();

	return *this;
}

Logger& Logger::operator<<(const QPoint& point)
{
	(*this) << "Point(" << point.x() << "," << point.y() << ")";
	return *this;
}

Logger& Logger::operator<<(const QSize& size)
{
	(*this) << "Size(" << size.width() << "," << size.height() << ")";
	return *this;
}

Logger& Logger::operator<<(const QRect& r)
{
	(*this) << "Rect("
	        << "left:" << r.left()
	        << ", right:" << r.right()
	        << ", top:" << r.top()
	        << ", bottom:" << r.bottom()
	        << ", width:" << r.width()
	        << ", heigh:" << r.height();
	return *this;
}

Logger& Logger::operator<<(const QByteArray& arr)
{
	m->msg << std::endl;

	QString line_str;

	for(int i = 0; i < arr.size(); i++)
	{
		char c = arr[i];

		QChar qc = QChar(c);

		if(qc.isPrint())
		{
			line_str += qc;
		}

		else
		{
			line_str += ".";
		}

		m->msg << std::hex << (unsigned int) (c & (0xff)) << " ";

		if(i % 8 == 7)
		{
			m->msg << "\t";
			m->msg << line_str.toLocal8Bit().constData() << std::endl;

			line_str.clear();
		}
	}

	if(!line_str.isEmpty())
	{
		for(int i = 0; i < 8 - line_str.size(); i++)
		{
			m->msg << "   ";
		}

		m->msg << "\t" << line_str.toLocal8Bit().constData() << std::endl;
	}

	return *this;
}

Logger& Logger::operator<<(const char* str)
{
	m->msg << str;

	return *this;
}

Logger& Logger::operator<<(const std::string& str)
{
	(*this) << str.c_str();
	return *this;
}

/*************************
 * Static Log functions
 * ***********************/
Logger spLog(const Log& type, const std::string& data)
{
	QString class_name;
	if(!data.empty())
	{
#ifdef SAYONARA_HAS_CXX_ABI
		int status;
		char* content = abi::__cxa_demangle(data.c_str(), nullptr, nullptr, &status);
		if(content && strnlen(content, 3) > 1)
		{
			class_name = QString(content);
			free(content);
		}
		else
		{
			class_name = QString::fromStdString(data);
		}

#else
		class_name = QString::fromStdString(data);
#endif
	}

	return Logger(type, class_name);
}

Logger spLog(const Log& type, const char* data)
{
	return spLog(type, std::string(data));
}
