/* InstanceChecker.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "InstanceChecker.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Parser/CommandLineParser.h"
#include "Utils/Utils.h"

#include <QByteArray>
#include <QDir>
#include <QSharedMemory>
#include <QStringList>

#include <cstring>

namespace
{
	constexpr const auto MemorySize = 16384;
	
	bool attachMemory(QSharedMemory& memory)
	{
		return memory.isAttached() || (memory.attach() && (memory.data() != nullptr));
	}
	
	QByteArray createRequestWithFilelist(const QStringList& fileList)
	{
		auto data = QByteArray("ReqD") + fileList.join('\n').toUtf8();
		const auto newSize = (data.size() >= MemorySize)
			? MemorySize
			: data.size() + 1;

		data.resize(newSize);
		data[newSize - 1] = '\0';

		return data;
	}

	QByteArray createEmptyRequest()
	{
		return QByteArray("ReqE");
	}

	void writeToMemory(const QByteArray& data, QSharedMemory& memory)
	{
		if(attachMemory(memory) && memory.data())
		{
			memory.lock();
			memcpy(memory.data(), data.constData(), std::min(data.size(), memory.size()));
			memory.unlock();
		}
	}

	bool waitForResponse(const QSharedMemory& memory)
	{
		Util::sleepMs(500);

		return (memory.constData())
			? (memcmp(memory.constData(), "Ack", 3) == 0)
			: false;
	}

	bool sendRequest(const CommandLineData& commandLineData, QSharedMemory& memory)
	{
		const auto data = (!commandLineData.filesToPlay.isEmpty())
		                  ? createRequestWithFilelist(commandLineData.filesToPlay)
		                  : createEmptyRequest();

		writeToMemory(data, memory);

		return waitForResponse(memory);
	}

	void fillSharedMemory(QSharedMemory& memory)
	{
		if(attachMemory(memory))
		{
			writeToMemory("Sayonara", memory);
		}
	}

	bool createMemory(QSharedMemory& memory)
	{
		if(!memory.create(MemorySize, QSharedMemory::ReadWrite))
		{
			const auto error = memory.errorString();
			spLog(Log::Debug, "Create memory") << error;
		}

		return (memory.error() == QSharedMemory::NoError);
	}
}

struct InstanceChecker::Private
{
	QSharedMemory sharedMemory{QString("SayonaraMemory%1").arg(QDir::homePath())};
};

InstanceChecker::InstanceChecker()
{
	m = Pimpl::make<Private>();
}

InstanceChecker::~InstanceChecker() = default;

bool InstanceChecker::isCurrentInstanceAllowed(const CommandLineData& commandLineData)
{
	spLog(Log::Always, this) << "Check for another instance...";

	if(commandLineData.multipleInstances)
	{
		createMemory(m->sharedMemory);
		fillSharedMemory(m->sharedMemory);

		return true;
	}

	if(createMemory(m->sharedMemory))
	{
		fillSharedMemory(m->sharedMemory);
		return true;
	}

	if(!attachMemory(m->sharedMemory))
	{
		spLog(Log::Always, this) << "Can't attach shared memory. Ignoring other instance.";
		return true;
	}

	const auto responseReceived = sendRequest(commandLineData, m->sharedMemory);
	if(responseReceived)
	{
		spLog(Log::Always, this) << "Another instance is running";
		m->sharedMemory.detach();
		return false;
	}

	spLog(Log::Always, this) << "No response from other instance. Ignoring other instance.";
	fillSharedMemory(m->sharedMemory);
	return true;
}


