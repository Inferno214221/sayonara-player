/* ArtistMatch.h */

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
				double similarity{-1.0};

				Entry() = default;
				Entry(const QString& artist, const QString& mbid, double similarity);

				bool isValid() const;

				bool operator==(const Entry& other) const;
			};

			/**
			 * @brief The Quality enum used to access the bin of interest. See ArtistMatch::get(Quality q)
			 */
			enum class Quality : uint8_t
			{
					Poor = 0,
					Good,
					VeryGood,
					Excellent
			};

			ArtistMatch();
			explicit ArtistMatch(const QString& artist_name);
			ArtistMatch(const ArtistMatch& other);

			virtual ~ArtistMatch();

			/**
			 * @brief checks, if structure is valid.
			 * @return false, if all bins are empty. True else
			 */
			bool isValid() const;

			/**
			 * @brief Compares two ArtistMatch structures
			 * @param am the other ArtistMatch
			 * @return true, if the artist string is the same. False else
			 */
			bool operator==(const ArtistMatch& am) const;
			ArtistMatch& operator=(const ArtistMatch& other);

			/**
			 * @brief adds an artist string to the corresponding bin
			 * @param artist artist string
			 * @param match the match value
			 */
			void add(const Entry& entry);

			/**
			 * @brief get bin by quality
			 * @param q quality. See ArtistMatch::Quality
			 * @return the desired bin
			 */
			QList<Entry> get(Quality q) const;

			/**
			 * @brief get the corresponding artist name of the ArtistMatch structure
			 * @return artist name
			 */
			QString artistName() const;

			/**
			 * @brief converts the artist match to string
			 * @return string representation
			 */
			QString toString() const;

			static ArtistMatch fromString(const QString& data);
	};
}

#endif // ARTISTMATCH_H
