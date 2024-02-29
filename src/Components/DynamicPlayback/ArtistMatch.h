/* ArtistMatch.h */

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

#ifndef ARTISTMATCH_H
#define ARTISTMATCH_H

#include "Utils/Pimpl.h"

#include <QList>
#include <QString>

namespace DynamicPlayback
{
	class ArtistMatch
	{
		PIMPL(ArtistMatch)

		public:
			struct Entry
			{
				QString artist;
				QString mbid;
				double similarity {-1.0};

				Entry() = default;
				Entry(QString artist, QString mbid, double similarity);

				[[nodiscard]] bool isValid() const;

				[[nodiscard]] bool operator==(const Entry& other) const;
			};

			enum class Quality :
				uint8_t
			{
				Poor = 0,
				Good,
				VeryGood,
				Excellent
			};

			ArtistMatch();
			explicit ArtistMatch(const QString& artistName);
			ArtistMatch(const ArtistMatch& other);

			virtual ~ArtistMatch();

			[[nodiscard]] bool isValid() const;

			[[nodiscard]] bool operator==(const ArtistMatch& artistMatch) const;
			ArtistMatch& operator=(const ArtistMatch& other);

			void add(const Entry& entry);

			[[nodiscard]] QList<Entry> get(Quality q) const;

			[[nodiscard]] QString artistName() const;

			[[nodiscard]] QString toString() const;

			static ArtistMatch fromString(const QString& data);
	};
}

#endif // ARTISTMATCH_H
