/* CoverLocation.h */

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

#ifndef COVERLOCATION_H
#define COVERLOCATION_H

#include "Utils/Pimpl.h"

#include <QMetaType>
#include <QString>

class QUrl;

namespace Cover
{
	namespace Fetcher
	{
		class Url;
	}

	class Location
	{
		PIMPL(Location)

		public:
			Location();
			~Location();
			Location(const Location& cl);
			Location(Location&& cl) noexcept;

			Location& operator=(const Location& cl);
			Location& operator=(Location&& cl) noexcept;

			bool isValid() const;

			QString hashPath() const;

			QString identifier() const;

			QList<Fetcher::Url> searchUrls() const;

			bool hasSearchUrls() const;

			QString searchTerm() const;

			void setSearchTerm(const QString& searchTerm,
			                   const QString& coverFetcherIdentifier = QString());

			void setSearchUrls(const QList<Fetcher::Url>& urls);

			void enableFreetextSearch(bool enabled);

			QString hash() const;

			bool hasAudioFileSource() const;

			QString audioFileSource() const;

			QString audioFileTarget() const;

			QString localPathDir() const;

			QStringList localPathHints() const;

			QString localPath() const;

			QString preferredPath() const;

			QString alternativePath() const;

			static Location coverLocation(const QString& albumName, const QString& artistName);

			static Location coverLocation(const QString& albumName, const QStringList& artists);

			static Location coverLocation(const QString& artist);

			static Location coverLocation(const Artist& artist);

			static Location coverLocation(const MetaData& track);

			static Location coverLocation(const QList<QUrl>& urls, const QString& token);

			static Location coverLocation(const Album& album);

			static Location invalidLocation();

			static QString invalidPath();

		private:
			void setValid(bool valid);
			void setIdentifier(const QString& identifier);
			void setLocalPathHints(const QStringList& localPaths);

			void setHash(const QString& str);

			bool setAudioFileSource(const QString& audioFileSource, const QString& symlinkPath);

			// must be here because attributes are private
			static Location coverLocationRadio(const QString& stationName, const QString& stationUrl,
			                                   const QStringList& coverDownloadUrls);
	};
}

Q_DECLARE_METATYPE(Cover::Location)

#endif // COVERLOCATION_H
