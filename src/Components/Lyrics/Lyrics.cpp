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
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QUrl>
#include <utility>

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
		Tagging::TagReaderPtr tagReader;
		Tagging::TagWriterPtr tagWriter;
		QStringList servers {::Lyrics::LookupThread().servers()};
		MetaData track;
		QString artist;
		QString title;
		QString lyrics;
		QString lyricHeader;
		QString lyricTagContent;
		
		Private(Tagging::TagReaderPtr tagReader, Tagging::TagWriterPtr tagWriter) :
			tagReader {std::move(tagReader)},
			tagWriter {std::move(tagWriter)} {}
	};

	Lyrics::Lyrics(const Tagging::TagReaderPtr& tagReader, const Tagging::TagWriterPtr& tagWriter, QObject* parent) :
		QObject(parent),
		m {Pimpl::make<Private>(tagReader, tagWriter)} {}

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

		const auto success = m->tagWriter->writeLyrics(m->track.filepath(), plainText);
		if(success)
		{
			m->lyricTagContent = plainText;
		}

		return success;
	}

	QStringList Lyrics::servers() const { return m->servers; }

	void Lyrics::setMetadata(const MetaData& track)
	{
		const auto [artist, title] = guessArtistAndTitle(track);
		m->artist = artist;
		m->title = title;
		m->track = track;

		if(const auto maybeLyrics = m->tagReader->extractLyrics(track.filepath()); maybeLyrics.has_value())
		{
			m->lyricTagContent = maybeLyrics.value();
			spLog(Log::Debug, this) << QString("Lyrics found in %1").arg(track.filepath());
		}

		else
		{
			m->lyricTagContent.clear();
			spLog(Log::Debug, this) << QString("Could not find lyrics in %1").arg(track.filepath());
		}
	}

	QString Lyrics::artist() const { return m->artist; }

	QString Lyrics::title() const { return m->title; }

	QString Lyrics::lyricHeader() const { return m->lyricHeader; }

	QString Lyrics::localLyricHeader() const
	{
		return QString("<b>%1 - %2</b>")
			.arg(artist())
			.arg(title());
	}

	QString Lyrics::lyrics() const { return m->lyrics.trimmed(); }

	QString Lyrics::localLyrics() const
	{
		return isLyricTagAvailable()
		       ? m->lyricTagContent.trimmed()
		       : QString();
	}

	bool Lyrics::isLyricTagAvailable() const { return (!m->lyricTagContent.isEmpty()); }

	bool Lyrics::isLyricTagSupported() const { return m->tagReader->isLyricsSupported(m->track.filepath()); }

	void Lyrics::lyricsFetched()
	{
		auto* lyricThread = dynamic_cast<::Lyrics::LookupThread*>(sender());

		m->lyrics = !lyricThread->hasError()
		            ? lyricThread->lyricData().trimmed()
		            : QString {};

		m->lyricHeader = lyricThread->lyricHeader();

		lyricThread->deleteLater();

		emit sigLyricsFetched();
	}
}
