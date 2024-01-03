/* CoverExtractor.cpp */

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

#include "CoverExtractor.h"
#include "CoverLocation.h"

#include "Utils/CoverUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QString>
#include <QPixmap>
#include <mutex>

namespace FileUtils = ::Util::File;
using Util::Covers::Source;
using LockGuard = std::lock_guard<std::mutex>;

namespace
{
	std::mutex mutexIo;
}

struct Cover::Extractor::Private
{
	QPixmap pixmap;
	Cover::Location coverLocation;
	Source source;

	Private(const Cover::Location& coverLocation) :
		coverLocation(coverLocation),
		source(Source::Unknown) {}
};

Cover::Extractor::Extractor(const Location& cl, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl);
}

Cover::Extractor::~Extractor() = default;

QPixmap Cover::Extractor::pixmap() const
{
	return m->pixmap;
}

Util::Covers::Source Cover::Extractor::source() const
{
	return m->source;
}

void Cover::Extractor::start()
{
	m->pixmap = QPixmap();

	{ // check for audio file target
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexIo);
		const auto audioFileTarget = m->coverLocation.audioFileTarget();
		if(FileUtils::exists(audioFileTarget))
		{
			m->pixmap = QPixmap(m->coverLocation.audioFileTarget());
			m->source = Source::AudioFile;
		}
	}

	// check sayonara path
	if(m->pixmap.isNull())
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexIo);
		const auto coverPath = m->coverLocation.hashPath();
		if(FileUtils::exists(coverPath))
		{
			m->pixmap = QPixmap(coverPath);
			m->source = Source::SayonaraDir;
		}
	}

	// check for audio file source
	if(m->pixmap.isNull())
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexIo);
		const auto audioFileSource = m->coverLocation.audioFileSource();
		if(FileUtils::exists(audioFileSource))
		{
			m->pixmap = Tagging::extractCover(audioFileSource);
			m->source = Source::AudioFile;
		}
	}

	// check for path in library dir
	if(m->pixmap.isNull())
	{
		[[maybe_unused]] const auto lockGuard = LockGuard(mutexIo);
		const auto localPath = m->coverLocation.localPath();
		if(FileUtils::exists(localPath))
		{
			m->pixmap = QPixmap(localPath);
			m->source = Source::Library;
		}
	}

	emit sigFinished();
}
