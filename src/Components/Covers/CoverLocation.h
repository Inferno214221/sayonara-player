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

#include <QMetaType>
#include "Utils/Pimpl.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"

class QUrl;

namespace Cover
{
	using StringMap=QMap<QString, QString>;

	/**
	 * @brief The CoverLocation class
	 * @ingroup Covers
	 */
	class Location
	{
		PIMPL(Location)

	private:
		void setValid(bool b);
		void setIdentifier(const QString& identifier);
		void setLocalPathHints(const QStringList& localPaths);

		/**
		 * @brief Set hash manually. You should never call this function
		 * @param str
		 */
		void			setHash(const QString& str);

		/**
		 * @brief Set the audio file source manually. You should not
		 * use this function from the outside
		 * @param audio_file_source
		 * @param coverPath
		 * @return true if everything is alright with audio_filesource oder coverPath (not empty)
		 */
		bool			setAudioFileSource(const QString& audio_filesource, const QString& symlinkPath);


	public:
		/**
		 * @brief Default constructor. Creates an invalid Location with
		 * the Sayonara logo as cover
		 */
		Location();
		~Location();
		Location(const Location& cl);
		Location& operator=(const Location& cl);

		/**
		 * @brief returns if the current location is a valid or
		 * a standard constructed location
		 * @return
		 */
		bool			isValid() const;

		/**
		 * @brief Returns the standard cover path in the .Sayonara
		 * directory
		 * @return
		 */
		QString			symlinkPath() const;

		/**
		 * @brief This identifier may be used in order to check
		 * how the cover algorithm determined the locations
		 * @return
		 */
		QString			identifer() const;

		/**
		 * @brief Retrieve the urls where a new cover can be searched
		 * @return
		 */
		QList<Fetcher::Url> searchUrls() const;


		/**
		 * @brief Check for existing search urls
		 * @return
		 */
		bool			hasSearchUrls() const;


		/**
		 * @brief Search term for a free search. As a human you would
		 * type that search term into your browser
		 * @return
		 */
		QString			searchTerm() const;

		/**
		 * @brief Set a new search term
		 * @param search_term
		 */
		void			setSearchTerm(const QString& searchTerm);

		/**
		 * @brief Set a new search term for a specific cover fetcher
		 * Cover fetcher strings can be found in the Cover::Fetcher::Base implementations
		 * by calling Cover::Fetcher::Base::identifier()
		 * @param search_term A searchterm suitable for the specific Cover::Fetcher::Base.
		 * For example "Master of puppets Metallica"
		 * @param cover_fetcher_identifier
		 */
		void			setSearchTerm(const QString& searchTerm,
										const QString& coverFetcherIdentifier);

		/**
		 * @brief Set urls where to look for Covers in the internet
		 * @param urls
		 */
		void			setSearchUrls(const QList<Fetcher::Url>& urls);

		/**
		 * @brief When enabling freetext search you specify the
		 * search string yourself and it is not generated automatically
		 * as usually. Usually, search APIs of a provider have special
		 * reserved fields for albums and artist which are populated
		 * automatically by Sayonara.
		 * @param b
		 */
		void			enableFreetextSearch(bool b);

			/**
			 * @brief to_string
			 * @return
			 */
		QString			toString() const;

		/**
		 * @brief Every combination of album and artist will result
		 * in a specific hash. You can find those hashes in the database,
		 * for example.
		 * @return
		 */
		QString			hash() const;

		/**
		 * @brief Indicates if it is possible to fetch the cover
		 * directly from the audio file. If you call this method
		 * very often for albums this may end up in poor performance
		 * @return
		 */
		bool			hasAudioFileSource() const;

		/**
		 * @brief Returns the path to the music file where a cover
		 * is stored. You can extract the cover by using
		 * Tagging::Covers::extract_cover
		 * @return
		 */
		QString			audioFileSource() const;

		/**
		 * @brief When retrieving the audio_file_source, and you want
		 * to save it afterwards, store it at the place returned by
		 * this method. So Cover::Lookup will find it. Also see hash()
		 * @return
		 */
		QString			audioFileTarget() const;



		/**
		 * @brief Calculates the directory where the cover is located
		 * @return
		 */
		QString			localPathDir() const;

		/**
		 * @brief Get the paths audio file where a cover is stored
		 * in the same directory
		 * @return
		 */
		QStringList		localPathHints() const;

		/**
		 * @brief Get the path which is nearest to the audio files.\n
		 * Repair/Create a link in the Sayonara cover directory
		 * This method does I/O work so handle with care
		 * @return
		 */
		QString			localPath() const;


		/**
		 * @brief Use this to retrieve a filepath where a copy of the
		 * cover is stored.
		 * @return
		 */
		QString			preferredPath() const;


		QString			alternativePath() const;


		/**
		 * @brief creates CoverLocation by taking the md5 sum between albumName and artistName
		 * @param albumName Album name
		 * @param artistName Artist name
		 * @return CoverLocation object
		 */
		static Location coverLocation(const QString& albumName, const QString& artistName);

		/**
		 * @brief overloaded. Picks major artist out of artists and calls
		 *   coverLocation(const QString& albumName, const QString& artistName)
		 * @param albumName Album name
		 * @param artists List of artists
		 * @return CoverLocation object
		 */
		static Location coverLocation(const QString& albumName, const QStringList& artists);


		/**
		 * @brief overloaded. Calls
		 *   coverLocation(const QString& albumName, const QStringList& artists)
		 * @param album
		 * @return CoverLocation object
		 */
		static Location xcoverLocation(const Album& album);


		/**
		 * @brief Creates cover token of the form artist_<md5sum of artist>
		 * @param artist Artist name
		 * @return CoverLocation object
		 */
		static Location coverLocation(const QString& artist);
		static Location coverLocationRadio(const QString& radioStation);


		/**
		 * @brief overloaded. extracts artist name and calls
		 *   coverLocation(const QString& artist)
		 * @param artist Artist object
		 * @return CoverLocation object
		 */
		static Location coverLocation(const Artist& artist);


		/**
		 * @brief overloaded.
		 *   if MetaData::albumId < 0 calls
		 *     coverLocation(const QString& albumName, const QString& artistName)
		 *   else extract Album from database and calls
		 *     coverLocation(const Album& album)
		 * @param Metadata object
		 * @return  CoverLocation object
		 */
		static Location coverLocation(const MetaData& md);
		static Location coverLocation(const MetaData& md, bool checkForCoverart);


		/**
		 * @brief fetch a cover from a specific url
		 * @param url url, the cover has to be fetched from
		 * @param targetPath path where the found image has to be saved
		 * @return CoverLocation object
		 */
		static Location coverLocation(const QList<QUrl>& urls, const QString& token);


		static QString invalidPath();

		/**
		 * @brief returns an invalid location
		 * @return  CoverLocation object
		 */
		static Location invalidLocation();
	};
}

Q_DECLARE_METATYPE(Cover::Location)

#endif // COVERLOCATION_H
