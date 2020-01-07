/* SoundcloudJsonParser.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef SOUNDCLOUDJSONPARSER_H
#define SOUNDCLOUDJSONPARSER_H

#include <QJsonObject>
#include <QJsonArray>
#include <QObject>

#include "Utils/Pimpl.h"

class MetaData;
class MetaDataList;
class Artist;
class Album;
class ArtistList;
class AlbumList;

class QByteArray;
namespace SC
{
	class JsonParser : public QObject
	{
		Q_OBJECT
		PIMPL(JsonParser)

	private:
		enum class SCJsonItemType : uint8_t
		{
			Track=0,
			Artist,
			Playlist
		};

	public:
		explicit JsonParser(const QByteArray& content);
		~JsonParser();

		bool	parse_artist_list(ArtistList& artists, QJsonArray arr);
		bool	parse_track_list(ArtistList& artists, MetaDataList& v_md, QJsonArray arr);
		bool	parse_playlist_list(ArtistList& artists, AlbumList& albums, MetaDataList& v_md, QJsonArray arr);

		bool	parse_artist(Artist& artist, QJsonObject object);
		bool	parse_playlist(ArtistList& artists, Album& album, MetaDataList& v_md, QJsonObject object);
		bool	parse_track(Artist& artist, MetaData& md, QJsonObject object);

		QString	create_link(const QString& name, const QString& target);

		bool	get_string(const QString& key, const QJsonObject& object, QString& str);
		bool	get_int(const QString& key, const QJsonObject& object, int& i);
		bool	get_array(const QString& key, const QJsonObject& object, QJsonArray& arr);
		bool	get_object(const QString& key, const QJsonObject& object, QJsonObject& o);

		bool	parse_artists(ArtistList& artists);
		bool	parse_tracks(ArtistList& artists, MetaDataList& v_md);
		bool	parse_playlists(ArtistList& artists, AlbumList& albums, MetaDataList& v_md);
	};
}

#endif // SOUNDCLOUDJSONPARSER_H
