/* CoverLocation.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include <QMap>
#include "Utils/Pimpl.h"

class QUrl;
class QString;
class QStringList;
class MetaData;
class Album;
class Artist;

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
		void set_valid(bool b);
		void set_identifier(const QString& identifier);
		void set_cover_path(const QString& cover_path);


	public:

		enum CoverSourceType
		{
			Invalid=0,
			SayonaraCoverDir,
			LocalPath,
			AudioFile
		};

		Location();
		~Location();
		Location(const Location& cl);
		Location& operator=(const Location& cl);

		/**
		 * @brief returns if the current location is a valid or
		 * a standard constructed location
		 * @return
		 */
		bool			valid() const;

		/**
		 * @brief returns if path is the same as the invalid location path
		 * @param cover_path the path to be compared
		 * @return
		 */
		static bool is_invalid(const QString& cover_path);


		/**
		 * @brief returns paths of images in the directory
		 * where the soundfiles are located.
		 * @return
		 */
		QStringList		local_paths() const;

		/**
		 * @brief adds an local path to other local paths
		 * @param path new path
		 */
		void			add_local_path(const QString& path);

		/**
		 * @brief Returns the most suitable path for covers.
		 * Usually this is the first of local_paths(). If not,
		 * the cover_path() is taken. If this file does not exist
		 * either, the fallback path is returned (logo)
		 * @return
		 */
		QString			preferred_path() const;

		CoverSourceType get_cover_source_type() const;

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
		const QStringList& search_urls() const;

		QString search_url(int idx) const;

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

		void			set_search_term(const QString& search_term,
										const QString& cover_fetcher_identifier);

		void			set_search_urls(const QStringList& urls);

		void			enable_freetext_search(bool b);
		bool			is_freetext_search_enabled() const;

		/**
		 * @brief to_string
		 * @return
		 */
		QString			to_string() const;


		bool			has_audio_file_source() const;

		QString			audio_file_source() const;

		void			set_audio_file_source(const QString& audio_file_source, const QString& cover_path);


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
		static Location cover_location(const Album& album);


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


		/**
		 * @brief fetch a cover from a specific url
		 * @param url url, the cover has to be fetched from
		 * @param target_path path where the found image has to be saved
		 * @return CoverLocation object
		 */
		static Location cover_location(const QUrl& url, const QString& target_path);


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
