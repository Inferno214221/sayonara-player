/* SomaFMStation.h */

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


/* SomaFMStation.h */

#ifndef SOMAFMSTATION_H
#define SOMAFMSTATION_H

#include "Utils/Pimpl.h"

class QStringList;
class QString;
class MetaDataList;

namespace Cover
{
    class Location;
}

namespace SomaFM
{
	class Station
	{
		PIMPL(Station)

		public:
			enum class UrlType : unsigned char
			{
				AAC=0,
				MP3,
				Undefined
			};

			Station();
			explicit Station(const QString& content);
			Station(const Station& other);
			Station& operator=(const Station& other);
			~Station();

			QString			name() const;
			QStringList		playlists() const;
			QString			description() const;
			UrlType			urlType(const QString& url) const;
			Cover::Location coverLocation() const;
			bool			isValid() const;

			MetaDataList metadata() const;
			void setMetadata(const MetaDataList& tracks);

			bool isLoved() const;
			void setLoved(bool loved);

		private:
			void parseStationName();
			void parseUrls();
			void parseDescription();
			void parseImage();
	};
}

#endif

