
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

#include "InstanceThread.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QByteArray>
#include <QDir>

namespace
{
	bool attachMemory(QSharedMemory& memory)
	{
		return (memory.isAttached() || memory.attach());
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

	QByteArray readMemory(QSharedMemory& memory, int offset, int maxBytes)
	{
		if(attachMemory(memory) && memory.data())
		{
			offset = std::min(memory.size() - 1, offset);
			maxBytes = std::min(memory.size() - offset, maxBytes);

			const auto* data = static_cast<const char*>(memory.constData()) + offset;

			return (data && (maxBytes > 0))
			       ? QByteArray(data, maxBytes)
			       : QByteArray();
		}

		return QByteArray();
	}

	bool isFilepathValid(const QString& filepath)
	{
		return (!filepath.isEmpty()) &&
		       (Util::File::isPlaylistFile(filepath) || Util::File::isSoundFile(filepath));
	}

	QStringList parseMemory(QSharedMemory& memory)
	{
		QStringList result;

		const auto data = readMemory(memory, 4, memory.size() - 5);
		if(!data.isNull())
		{
			spLog(Log::Info, "InstanceThread") << data;
			const auto filepaths = data.split('\n');
			for(const auto& filepathData: filepaths)
			{
				const auto filepath = QString::fromUtf8(filepathData);
				if(isFilepathValid(filepath))
				{
					spLog(Log::Debug, "FromSharedMemory") << "Add file " << filepath;
					result << filepath;
				}
			}
		}

		return result;
	}
}

struct InstanceThread::Private
{
	QSharedMemory memory {QByteArray("SayonaraMemory") + QDir::homePath()};
	QStringList paths;
	bool mayRun {true};
};

InstanceThread::InstanceThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
}

InstanceThread::~InstanceThread()
{
	m->memory.detach();
}

void InstanceThread::run()
{
	if(!attachMemory(m->memory))
	{
		return;
	}

	while(m->mayRun)
	{
		const auto data = readMemory(m->memory, 0, 4);
		if((data.size() == 4) && data.startsWith("Req"))
		{
			spLog(Log::Info, this) << "Second instance saying hello";

			if(data[3] == 'D')
			{
				m->paths = parseMemory(m->memory);
				emit sigCreatePlaylist();
			}

			writeToMemory("Ack", m->memory);
			emit sigPlayerRise();
		}

		if(m->mayRun)
		{
			Util::sleepMs(100);
		}
	}
}

void InstanceThread::stop()
{
	m->mayRun = false;
}

QStringList InstanceThread::paths() const
{
	return m->paths;
}
