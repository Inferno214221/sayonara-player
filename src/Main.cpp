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
#include "Application/InstanceChecker.h"

#include "Utils/Parser/CommandLineParser.h"
#include "Utils/StandardPaths.h"

#include "Utils/Logger/Logger.h"

#ifdef SAYONARA_HAS_BACKTRACE
#include <execinfo.h>
#include <csignal>
#include <unistd.h>
#endif

namespace
{
#ifdef SAYONARA_HAS_BACKTRACE

	void segfaultHandler([[maybe_unused]] int sig)
	{
		std::array<void*, 20> backtraceData;
		// get void*'s for all entries on the stack
		const auto size = backtrace(backtraceData.data(), 20);
		backtrace_symbols_fd(backtraceData.data(), size, STDERR_FILENO);
	}

#endif
}

int main(int argc, char* argv[])
{
	const auto commandLineData = CommandLineParser::parse(argc, argv);
	if(commandLineData.abort)
	{
		return 0;
	}

	InstanceChecker instanceChecker;
	if(!instanceChecker.isCurrentInstanceAllowed(commandLineData))
	{
		return 0;
	}

	// do not access DB or any of the config/share/cache standard paths before this line
	auto app = Application(argc, argv);

	spLog(Log::Info, "Sayonara") << "Config path: " << Util::xdgConfigPath();
	spLog(Log::Info, "Sayonara") << "Share path: " << Util::xdgSharePath();

#ifdef SAYONARA_HAS_BACKTRACE
	signal(SIGSEGV, segfaultHandler);
#endif

	if(!app.init(commandLineData.filesToPlay, commandLineData.forceShow))
	{
		return 1;
	}

	return app.exec();
}
