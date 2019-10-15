/* CoverLocation.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

class QUrl;

namespace Cover
{
	namespace Fetcher
	{
		class Url;
	}

	using StringMap=QMap<QString, QString>;

	/**
	 * @brief The CoverLocation class
	 * @ingroup Covers
	 */
	class Location
	{
		PIMPL(Location)

	private:
		void set_valid(bool b);
		void set_identifier(const QString& identifier);
		void set_cover_path(const QString& cover_path);
		void set_local_path_hints(const QStringList& local_paths);

		/**
		 * @brief Set hash manually. You should never call this function
		 * @param str
		 */
		void			set_hash(const QString& str);

		/**
		 * @brief Set the audio file source manually. You should not
		 * use this function from the outside
		 * @param audio_file_source
		 * @param cover_path
		 * @return true if everything is alright with audio_filesource oder cover_path (not empty)
		 */
		bool			set_audio_file_source(const QString& audio_filesource, const QString& cover_path);


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
		bool			is_valid() const;


		/**
		 * @brief Returns the standard cover path in the .Sayonara
		 * directory
		 * @return
		 */
		QString			cover_path() const;

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
		QList<Fetcher::Url> search_urls(bool also_inactive) const;

		/**
		 * @brief Search urls contains urls from
		 * Google, Discogs or Audioscrobbler. They are
		 * ordered as configured in the Cover preferences
		 * Dialog
		 * @param idx
		 * @return
		 */
		Fetcher::Url search_url(int idx) const;


		/**
		 * @brief Check for existing search urls
		 * @return
		 */
		bool			has_search_urls() const;


		/**
		 * @brief Search term for a free search. As a human you would
		 * type that search term into your browser
		 * @return
		 */
		QString			search_term() const;

		/**
		 * @brief Set a new search term
		 * @param search_term
		 */
		void			set_search_term(const QString& search_term);

		/**
		 * @brief Set a new search term for a specific cover fetcher
		 * Cover fetcher strings can be found in the Cover::Fetcher::Base implementations
		 * by calling Cover::Fetcher::Base::identifier()
		 * @param search_term A searchterm suitable for the specific Cover::Fetcher::Base.
		 * For example "Master of puppets Metallica"
		 * @param cover_fetcher_identifier
		 */
		void			set_search_term(const QString& search_term,
										const QString& cover_fetcher_identifier);

		/**
		 * @brief Set urls where to look for Covers in the internet
		 * @param urls
		 */
		void			set_search_urls(const QList<Fetcher::Url>& urls);

		/**
		 * @brief When enabling freetext search you specify the
		 * search string yourself and it is not generated automatically
		 * as usually. Usually, search APIs of a provider have special
		 * reserved fields for albums and artist which are populated
		 * automatically by Sayonara.
		 * @param b
		 */
		void			enable_freetext_search(bool b);
		bool			is_freetext_search_enabled() const;

		/**
		 * @brief to_string
		 * @return
		 */
		QString			to_string() const;

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
		bool			has_audio_file_source() const;

		/**
		 * @brief Returns the path to the music file where a cover
		 * is stored. You can extract the cover by using
		 * Tagging::Covers::extract_cover
		 * @return
		 */
		QString			audio_file_source() const;

		/**
		 * @brief When retrieving the audio_file_source, and you want
		 * to save it afterwards, store it at the place returned by
		 * this method. So Cover::Lookup will find it. Also see hash()
		 * @return
		 */
		QString			audio_file_target() const;



		/**
		 * @brief Calculates the directory where the cover is located
		 * @return
		 */
		QString			local_path_dir() const;

		/**
		 * @brief Get the paths audio file where a cover is stored
		 * in the same directory
		 * @return
		 */
		QStringList		local_path_hints() const;

		/**
		 * @brief Get the path which is nearest to the audio files.\n
		 * Repair/Create a link in the Sayonara cover directory
		 * This method does I/O work so handle with care
		 * @return
		 */
		QString			local_path() const;


		/**
		 * @brief Use this to retrieve a filepath where a copy of the
		 * cover is stored.
		 * @return
		 */
		QString			preferred_path() const;


		QString			alternative_path() const;


		/**
		 * @brief creates CoverLocation by taking the md5 sum between album_name and artist_name
		 * @param album_name Album name
		 * @param artist_name Artist name
		 * @return CoverLocation object
		 */
		static Location cover_location(const QString& album_name, const QString& artist_name);

		/**
		 * @brief overloaded. Picks major artist out of artists and calls
		 *   cover_location(const QString& album_name, const QString& artist_name)
		 * @param album_name Album name
		 * @param artists List of artists
		 * @return CoverLocation object
		 */
		static Location cover_location(const QString& album_name, const QStringList& artists);


		/**
		 * @brief overloaded. Calls
		 *   cover_location(const QString& album_name, const QStringList& artists)
		 * @param album
		 * @return CoverLocation object
		 */
		static Location xcover_location(const Album& album);


		/**
		 * @brief Creates cover token of the form artist_<md5sum of artist>
		 * @param artist Artist name
		 * @return CoverLocation object
		 */
		static Location cover_location(const QString& artist);


		/**
		 * @brief overloaded. extracts artist name and calls
		 *   cover_location(const QString& artist)
		 * @param artist Artist object
		 * @return CoverLocation object
		 */
		static Location cover_location(const Artist& artist);


		/**
		 * @brief overloaded.
		 *   if MetaData::album_id < 0 calls
		 *     cover_location(const QString& album_name, const QString& artist_name)
		 *   else extract Album from database and calls
		 *     cover_location(const Album& album)
		 * @param Metadata object
		 * @return  CoverLocation object
		 */
		static Location cover_location(const MetaData& md);
		static Location cover_location(const MetaData& md, bool check_for_coverart);


		/**
		 * @brief fetch a cover from a specific url
		 * @param url url, the cover has to be fetched from
		 * @param target_path path where the found image has to be saved
		 * @return CoverLocation object
		 */
		static Location cover_location(const QUrl& url, const QString& target_path);
		static Location cover_location(const QList<QUrl>& urls, const QString& target_path);


		static QString invalid_path();

		/**
		 * @brief returns an invalid location
		 * @return  CoverLocation object
		 */
		static Location invalid_location();


		/**
		 * @brief returns the standard cover directory
		 * @return usually ~/.Sayonara/covers
		 */
		static QString get_cover_directory(const QString& append_path);
	};
}

Q_DECLARE_METATYPE(Cover::Location)

#endif // COVERLOCATION_H
