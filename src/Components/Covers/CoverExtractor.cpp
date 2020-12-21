/* CoverExtractor.cpp */

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

#include "CoverExtractor.h"
#include "CoverLocation.h"

#include "Utils/CoverUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/Mutex.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/TaggingCover.h"

#include <QString>
#include <QPixmap>

static std::mutex mutexIo;

namespace FileUtils = ::Util::File;
using Util::Covers::Source;

struct Cover::Extractor::Private
{
	QPixmap pixmap;
	Cover::Location cl;
	Source source;

	Private(const Cover::Location& cl) :
		cl(cl),
		source(Source::Unknown)
	{}
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
		LOCK_GUARD(mutexIo)
		QString audio_file_target = m->cl.audioFileTarget();
		if(FileUtils::exists(audio_file_target))
		{
			m->pixmap = QPixmap(m->cl.audioFileTarget());
			m->source = Source::AudioFile;
		}
	}

	// check sayonara path
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutexIo)
		QString cover_path = m->cl.hashPath();
		if(FileUtils::exists(cover_path))
		{
			m->pixmap = QPixmap(cover_path);
			m->source = Source::SayonaraDir;
		}
	}

	// check for audio file source
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutexIo)
		QString audio_file_source = m->cl.audioFileSource();
		if(FileUtils::exists(audio_file_source))
		{
			m->pixmap = Tagging::Covers::extractCover(audio_file_source);
			m->source = Source::AudioFile;
		}
	}

	// check for path in library dir
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutexIo)
		QString local_path = m->cl.localPath();
		if(FileUtils::exists(local_path))
		{
			m->pixmap = QPixmap(local_path);
			m->source = Source::Library;
		}
	}

	emit sigFinished();
}
