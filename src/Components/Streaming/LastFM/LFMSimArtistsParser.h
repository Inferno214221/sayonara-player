/* LFMSimArtistsParser.h */

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

#ifndef LFMSIMARTISTSPARSER_H
#define LFMSIMARTISTSPARSER_H

#include "Utils/Pimpl.h"

namespace LastFM
{
	class ArtistMatch;
	class SimArtistsParser
	{
		PIMPL(SimArtistsParser)

	public:
		SimArtistsParser(const QString& artist_name, const QByteArray& arr);
		SimArtistsParser(const QString& artist_name, const QString& filename);
		~SimArtistsParser();

		LastFM::ArtistMatch artistMatch() const;

	private:
		void parseDocument();
	};
}

#endif // LFMSIMARTISTSPARSER_H
