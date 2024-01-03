/* Lyrics.h */

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

#ifndef LYRICS_H
#define LYRICS_H

#include "Utils/Pimpl.h"

#include <QObject>

namespace Lyrics
{
	class Lyrics :
		public QObject
	{
		Q_OBJECT
		PIMPL(Lyrics)

		signals:
			void sigLyricsFetched();

		public:
			explicit Lyrics(QObject* parent = nullptr);
			~Lyrics();

			QStringList servers() const;
			void setMetadata(const MetaData& track);
			bool fetchLyrics(const QString& artist, const QString& title, int serverIndex);
			bool saveLyrics(const QString& plainText);

			QString artist() const;
			QString title() const;
			QString lyricHeader() const;
			QString localLyricHeader() const;
			QString lyrics() const;
			QString localLyrics() const;

			bool isLyricValid() const;
			bool isLyricTagAvailable() const;
			bool isLyricTagSupported() const;

		private slots:
			void lyricsFetched();
	};
}

#endif // LYRICS_H
