/* CoverExtractor.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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

#include <QString>
#include <QPixmap>

#include "Utils/FileUtils.h"
#include "Utils/Mutex.h"

#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/TaggingCover.h"

static std::mutex mutex_io;

namespace FileUtils=::Util::File;

struct Cover::Extractor::Private
{
	QPixmap pixmap;
	Cover::Location cl;

	Private(const Cover::Location& cl) :
		cl(cl)
	{}
};

Cover::Extractor::Extractor(const Location& cl, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(cl);

}

Cover::Extractor::~Extractor() = default;

QPixmap Cover::Extractor::pixmap()
{
	return m->pixmap;
}

void Cover::Extractor::start()
{
	m->pixmap = QPixmap();

	{ // check for audio file target
		LOCK_GUARD(mutex_io)
		QString audio_file_target = m->cl.audio_file_target();
		if(FileUtils::exists(audio_file_target))
		{
			m->pixmap = QPixmap(m->cl.audio_file_target());
		}
	}

	// check sayonara path
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString cover_path = m->cl.cover_path();
		if(FileUtils::exists(cover_path))
		{
			m->pixmap = QPixmap(cover_path);
		}
	}

	// check for audio file source
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString audio_file_source = m->cl.audio_file_source();
		if(FileUtils::exists(audio_file_source))
		{
			m->pixmap = Tagging::Covers::extract_cover(audio_file_source);
		}
	}

	// check for path in library dir
	if(m->pixmap.isNull())
	{
		LOCK_GUARD(mutex_io)
		QString local_path = m->cl.local_path();
		if(FileUtils::exists(local_path))
		{
			m->pixmap = QPixmap(local_path);
		}
	}

	emit sig_finished();
}
