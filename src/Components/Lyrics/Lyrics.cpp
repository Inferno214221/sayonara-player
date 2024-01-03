/* Lyrics.cpp */

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

#include "Lyrics.h"
#include "LyricLookup.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/TaggingLyrics.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QUrl>

namespace
{
	std::pair<QString, QString> guessArtistAndTitle(const MetaData& track)
	{
		auto artist = track.artist();
		auto title = track.title();

		if(track.radioMode() == RadioMode::Station)
		{
			if(track.title().contains(":"))
			{
				auto splitted = track.title().split(":");
				artist = splitted.takeFirst().trimmed();
				title = splitted.join(":").trimmed();
			}

			else if(track.title().contains("-"))
			{
				auto splitted = track.title().split("-");
				artist = splitted.takeFirst().trimmed();
				title = splitted.join("-").trimmed();
			}

			else if(!QUrl(artist).scheme().isEmpty())
			{
				artist.clear();
			}
		}

		return {artist, title};
	}
}

namespace Lyrics
{
	struct Lyrics::Private
	{
		QStringList servers;
		MetaData track;
		QString artist;
		QString title;
		QString lyrics;
		QString lyricHeader;
		QString lyricTagContent;

		bool isValid {false};

		Private() :
			servers(::Lyrics::LookupThread().servers()) {}
	};

	Lyrics::Lyrics(QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>()} {}

	Lyrics::~Lyrics() = default;

	bool Lyrics::fetchLyrics(const QString& artist, const QString& title, int serverIndex)
	{
		if(artist.isEmpty() || title.isEmpty())
		{
			return false;
		}

		if((serverIndex < 0) || (serverIndex >= m->servers.size()))
		{
			return false;
		}

		auto* lyricThread = new LookupThread(this);
		connect(lyricThread, &LookupThread::sigFinished, this, &Lyrics::lyricsFetched);

		lyricThread->run(artist, title, serverIndex);
		return true;
	}

	bool Lyrics::saveLyrics(const QString& plainText)
	{
		if(plainText.isEmpty() || m->track.filepath().isEmpty())
		{
			return false;
		}

		m->isValid = Tagging::writeLyrics(m->track, plainText);
		if(m->isValid)
		{
			m->lyricTagContent = plainText;
		}

		return m->isValid;
	}

	QStringList Lyrics::servers() const
	{
		return m->servers;
	}

	void Lyrics::setMetadata(const MetaData& track)
	{
		const auto [artist, title] = guessArtistAndTitle(track);
		m->artist = artist;
		m->title = title;
		m->track = track;

		const auto hasLyrics = Tagging::extractLyrics(track, m->lyricTagContent);
		const auto logString = (hasLyrics)
		                       ? QString("Could not find lyrics in %1").arg(track.filepath())
		                       : QString("Lyrics found in %1").arg(track.filepath());

		spLog(Log::Debug, this) << logString;
	}

	QString Lyrics::artist() const
	{
		return m->artist;
	}

	QString Lyrics::title() const
	{
		return m->title;
	}

	QString Lyrics::lyricHeader() const
	{
		return m->lyricHeader;
	}

	QString Lyrics::localLyricHeader() const
	{
		return QString("<b>%1 - %2</b>")
			.arg(artist())
			.arg(title());
	}

	QString Lyrics::lyrics() const
	{
		return m->lyrics.trimmed();
	}

	QString Lyrics::localLyrics() const
	{
		return isLyricTagAvailable()
		       ? m->lyricTagContent.trimmed()
		       : QString();
	}

	bool Lyrics::isLyricValid() const
	{
		return m->isValid;
	}

	bool Lyrics::isLyricTagAvailable() const
	{
		return (!m->lyricTagContent.isEmpty());
	}

	bool Lyrics::isLyricTagSupported() const
	{
		return Tagging::isLyricsSupported(m->track.filepath());
	}

	void Lyrics::lyricsFetched()
	{
		auto* lyricThread = static_cast<::Lyrics::LookupThread*>(sender());

		m->lyrics = lyricThread->lyricData();
		m->lyricHeader = lyricThread->lyricHeader();
		m->isValid = (!lyricThread->hasError());

		lyricThread->deleteLater();

		emit sigLyricsFetched();
	}
}