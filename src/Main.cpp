/* Main.cpp */

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

/*
 * Main.cpp
 *
 *  Created on: Mar 2, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include "Application/Application.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Parser/CommandLineParser.h"
#include "Utils/Logger/Logger.h"
#include "Database/Connector.h"

#include <QSharedMemory>
#include <QTranslator>
#include <QFontDatabase>
#include <QtGlobal>
#include <QDir>
#include <QIcon>
#include <algorithm>

namespace FileUtils=::Util::File;

static const int MemorySize=16384;

#ifdef Q_OS_UNIX
	#include <execinfo.h>		// backtrace
	#include <csignal>			// kill/signal
	#include <sys/types.h>		// kill
	#include <cstring>			// memcpy
	#include <unistd.h>			// STDERR_FILENO
#else

	#include <glib-2.0/glib.h>
	#undef signals
	#include <gio/gio.h>

#endif

#ifdef Q_OS_WIN
void init_gio()
{
	QString gio_path = FileUtils::clean_filename(QApplication::applicationDirPath()) + "/gio-modules";
	QString gst_plugin_path = FileUtils::clean_filename(QApplication::applicationDirPath()) + "/gstreamer-1.0/";

	Util::set_environment("GST_PLUGIN_PATH", gst_plugin_path);
	Util::set_environment("GST_PLUGIN_SYSTEM_PATH", gst_plugin_path);

	g_io_extension_point_register("gio-tls-backend");
	g_io_modules_load_all_in_directory(gio_path.toLocal8Bit().data());

	sp_log(Log::Debug, this) << "Done " << gio_path;
}
#endif

#include <stdio.h>

void segfault_handler(int sig)
{
	Q_UNUSED(sig)

#ifdef Q_OS_UNIX
	void* array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 20);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif

}

bool check_for_other_instance(const CommandLineData& cmd_data, QSharedMemory* memory)
{
	spLog(Log::Debug, "Main") << "Check for another instance";
	if(memory->create(MemorySize, QSharedMemory::ReadWrite))
	{
		spLog(Log::Debug, "Main") << "Could create memory";
		return false;
	}

	memory->attach(QSharedMemory::ReadWrite);
	if(!memory->data())
	{
		spLog(Log::Debug, "Main") << "No shared memory data";
		return false;
	}

	QByteArray data("Req");
	int size = MemorySize;
	Byte* ptr = static_cast<Byte*>(memory->data());

	if(!cmd_data.filesToPlay.isEmpty())
	{
		data += 'D';
		data += cmd_data.filesToPlay.join('\n').toUtf8();

		size = std::min(data.size(), MemorySize - 1);
		if(size < data.size()){
			data.resize(MemorySize);
		}

		data[MemorySize-1] = '\0';
	}

	else {
		data += "E";
	}

	memory->lock();
	spLog(Log::Debug, "Main") << "Sending to shared memory: " << data;
	memcpy(ptr, data.data(), size_t(data.size()));
	memory->unlock();

	Util::sleepMs(500);

	if(memcmp(memory->data(), "Ack", 3) == 0)
	{
		spLog(Log::Debug, "Main") << "There's probably another instance running";
		memory->detach();

		return true;
	}

	else {
		spLog(Log::Debug, "Main") << "Other instance not responding";
	}

	return false;
}


int main(int argc, char *argv[])
{
	Application app(argc, argv);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

	CommandLineData cmd_data = CommandLineParser::parse(argc, argv);
	if(cmd_data.abort){
		return 0;
	}

	QByteArray key("SayonaraMemory");
	key += QDir::homePath();

	QSharedMemory memory(key);

	if(!cmd_data.multipleInstances)
	{
		bool has_other_instance = check_for_other_instance(cmd_data, &memory);
		if(has_other_instance){
			return 0;
		}

		if(memory.data())
		{
			bool success = memory.attach(QSharedMemory::ReadWrite);
			spLog(Log::Debug, "Main") << "Attach memory " << success;
			if(!success){
				spLog(Log::Debug, "Main") << "Cannot attach memory " << memory.error() << ": " << memory.errorString();
			}
			memory.lock();
			memcpy(memory.data(), "Sayonara", 8);
			memory.unlock();
		}
	}

	QString sayonara_path = Util::sayonaraPath();
	if(!QFile::exists(sayonara_path))
	{
		spLog(Log::Error, "Sayonara") << "Cannot find and create Sayonara path '" << Util::sayonaraPath() << "'. Leaving now.";
		return 1;
	}

	spLog(Log::Debug, "Sayonara") << "Sayonara home path: " << Util::sayonaraPath();
	spLog(Log::Debug, "Sayonara") << "Sayonara share path: " << Util::sharePath();
	spLog(Log::Debug, "Sayonara") << "Sayonara lib path: " << Util::libPath();

	DB::Connector::instance();

#ifdef Q_OS_WIN
	init_gio();
#endif

#ifdef Q_OS_UNIX
	signal(SIGSEGV, segfault_handler);
#endif

	if(!FileUtils::exists( Util::sayonaraPath() )) {
		QDir().mkdir( Util::sayonaraPath() );
	}

	if(!app.init(cmd_data.filesToPlay, cmd_data.forceShow)) {
		return 1;
	}

	app.exec();

	return 0;
}
