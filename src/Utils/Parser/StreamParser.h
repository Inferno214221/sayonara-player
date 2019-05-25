/* StreamParser.h */

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

#ifndef STREAMPARSER_H
#define STREAMPARSER_H

#include "Utils/Pimpl.h"
#include <QObject>

using PlaylistFiles=QStringList;

class StreamParser : public QObject
{
	Q_OBJECT
	PIMPL(StreamParser)

	signals:
		void sig_finished(bool);
		void sig_stopped();
		void sig_too_many_urls_found(int n_urls, int n_max_urls);

	public:
		explicit StreamParser(const QString& station_name=QString(), QObject* parent=nullptr);
		~StreamParser();

		void parse_stream(const QString& url);
		void parse_streams(const QStringList& urls);
		void set_cover_url(const QString& cover_url);
		void stop();

		MetaDataList metadata() const;

	private slots:
		void awa_finished();
		void icy_finished();

	private:
		/**
		 * @brief Writes a temporary playlist file into the file system which is parsed later
		 * @param data Raw data extracted from the website
		 * @return filename where the playlist has been written at
		 */
		QString write_playlist_file(const QByteArray& data) const;

		/**
		 * @brief Parse content out of website data.
		 * First, check if the data is podcast data.\n
		 * Second, check if the data is a playlist file\n
		 * Else, search for playlist files within the content.
		 *
		 * @param data Raw website data
		 * @return list of tracks found in the website data
		 */

		QPair<MetaDataList, PlaylistFiles> parse_content(const QByteArray& data) const;

		/**
		 * @brief Parse website for playlist files and streams
		 * @param arr website data
		 * @return metadata list of found streams and a list of urls with playlist files
		 */
		QPair<MetaDataList, PlaylistFiles> parse_website(const QByteArray& arr) const;

		/**
		 * @brief Sset up missing fields in metadata: album, artist, title and filepath\n
		 * @param md reference to a MetaData structure
		 * @param stream_url url used to fill album/artist/filepath
		 */
		void tag_metadata(MetaData& md, const QString& stream_url, const QString& cover_url=QString()) const;

		/**
		 * @brief Parse the next Url in the queue. These urls may come from
		 * parsed playlist files or by using parse_streams(const QStringList& urls)
		 * @return
		 */
		bool parse_next_url();
};

#endif
