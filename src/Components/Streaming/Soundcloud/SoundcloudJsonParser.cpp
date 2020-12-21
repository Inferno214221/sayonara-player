/* SoundcloudJsonParser.cpp */

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

#include "SoundcloudGlobal.h"
#include "SoundcloudJsonParser.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/StandardPaths.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>

struct SC::JsonParser::Private
{
	QJsonDocument		jsonDocument;
	QByteArray			content;
	QJsonParseError		error;

	Private(const QByteArray& content) :
		content(content)
	{
		jsonDocument = QJsonDocument::fromJson(content, &error);
	}
};

SC::JsonParser::JsonParser(const QByteArray& content) :
	QObject()
{
	m = Pimpl::make<Private>(content);
	QString targetFile = Util::tempPath("soundcloud.json");

	Util::File::writeFile(
		m->jsonDocument.toJson(QJsonDocument::Indented), targetFile
	);

	QJsonParseError::ParseError pe = m->error.error;
	if(pe != QJsonParseError::NoError){
		spLog(Log::Warning, this) << "Cannot parse json document: " << m->error.errorString();
	}
}

SC::JsonParser::~JsonParser() = default;

bool SC::JsonParser::parseArtists(ArtistList& artists)
{
	if(m->jsonDocument.isArray()){
		return parseArtistList(artists, m->jsonDocument.array());
	}

	else if(m->jsonDocument.isObject()){
		Artist artist;
		if(parseArtist(artist, m->jsonDocument.object())){
			artists << artist;
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::parseArtistList(ArtistList& artists, QJsonArray arr)
{
	artists.clear();

	for(auto it = arr.begin(); it != arr.end(); it++){
		QJsonValueRef ref = *it;
		if(ref.isObject()){
			Artist artist;
			if(parseArtist(artist, ref.toObject())){
				artists << artist;
			}
		}
	}

	return true;
}


bool SC::JsonParser::parseArtist(Artist& artist, QJsonObject object)
{
	QString cover_download_url;

	ArtistId artistId;
	if(getInt("id", object, artistId)) {
		artist.setId(artistId);
	}

	QString artist_name;
	getString("username", object, artist_name);
	artist.setName(artist_name);

	getString("avatar_url", object, cover_download_url);
	artist.setCoverDownloadUrls({cover_download_url});

	QString description, website, permalink;
	if(getString("website", object, website)){
		artist.addCustomField("website", tr("Website"), website);
	}

	if(getString("permalink", object, permalink)){
		artist.addCustomField("permalink", tr("Permalink Url"), permalink);
	}

	if(getString("description", object, description)){
		artist.addCustomField("description", Lang::get(Lang::About), description);
	}

	int followers=-1;
	int following=-1;
	getInt("followers_count", object, followers);
	getInt("followings_count", object, following);

	if(followers != -1 && following != -1){
		artist.addCustomField("followers_following", tr("Followers/Following"), QString::number(followers) + "/" + QString::number(following));
	}

	return (artist.id() > 0);
}


bool SC::JsonParser::parseTracks(ArtistList& artists, MetaDataList &v_md)
{
	if(!m->jsonDocument.isArray()){
		return false;
	}

	return parseTrackList(artists, v_md, m->jsonDocument.array());
}


bool SC::JsonParser::parseTrackList(ArtistList& artists, MetaDataList& tracks, QJsonArray arr){
	tracks.clear();

	for(auto it = arr.begin(); it != arr.end(); it++)
	{
		QJsonValueRef ref = *it;
		if(ref.isObject())
		{
			MetaData md;
			Artist artist;
			if(parseTrack(artist, md, ref.toObject()))
			{
				md.setTrackNumber(TrackNum(tracks.size() + 1));

				tracks << md;

				if(!artists.contains(artist.id()))
				{
					artists << artist;
				}
			}

			else{
				spLog(Log::Debug, this) << "Invalid md found";
			}
		}
	}

	return true;
}

bool SC::JsonParser::parseTrack(Artist& artist, MetaData& md, QJsonObject object)
{
	QString coverDownloadUrl;

	TrackID id;
	if(getInt("id", object, id)){
		md.setId(id);
	}

	getString("artwork_url", object, coverDownloadUrl);
	md.setCoverDownloadUrls({coverDownloadUrl});

	int length;
	if(getInt("duration", object, length)){
		md.setDurationMs(MilliSeconds(length));
	}

	int year;
	if(getInt("release_year", object, year)){
		md.setYear(Year(year));
	}

	int filesize;
	if(getInt("original_content_size", object, filesize)){
		 md.setFilesize(Filesize(filesize));
	}

	QString title;
	if(getString("title", object, title)){
		md.setTitle(title);
	}

	QString streamUrl;
	if(getString("stream_url", object, streamUrl)){
		md.setFilepath(streamUrl + '?' + CLIENT_ID_STR);
	}

	QString genre;
	if(getString("genre", object, genre)){
		md.addGenre(Genre(genre));
	}

	QString purchaseUrl;
	if(getString("purchase_url", object, purchaseUrl)){
		md.addCustomField("purchase_url", tr("Purchase Url"), createLink(purchaseUrl, purchaseUrl));
	}

	QJsonObject artistObject;
	if(getObject("user", object, artistObject))
	{
		if( parseArtist(artist, artistObject) )
		{
			md.setArtist(artist.name());
			md.setArtistId(artist.id());

			if(md.albumId() < 0)
			{
				md.setAlbumId(0);
				md.setAlbum(Lang::get(Lang::UnknownAlbum));
			}
		}
	}

	QString lastModifiedString;
	if(getString("last_modified", object, lastModifiedString))
	{
		QDateTime dt = QDateTime::fromString(lastModifiedString, Qt::DateFormat::ISODate);
		md.setModifiedDate(Util::dateToInt(dt));
	}

	QString createdString;
	if(getString("created_at", object, createdString))
	{
		QDateTime dt = QDateTime::fromString(createdString, Qt::DateFormat::ISODate);
		md.setCreatedDate(Util::dateToInt(dt));
	}

	QString description;
	if(getString("description", object, description))
	{
		md.setComment(description);
	}

	return (md.filepath().size() > 0 && md.id() > 0);
}


bool SC::JsonParser::parsePlaylists(ArtistList& artists, AlbumList &albums, MetaDataList &v_md)
{
	if(m->jsonDocument.isArray()){
		return parsePlaylistList(artists, albums, v_md, m->jsonDocument.array());
	}

	else if(m->jsonDocument.isObject()){
		Album album;
		if(parsePlaylist(artists, album, v_md, m->jsonDocument.object())){
			albums << album;
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::parsePlaylistList(ArtistList& artists, AlbumList& albums, MetaDataList& v_md, QJsonArray arr)
{
	albums.clear();

	for(auto it = arr.begin(); it != arr.end(); it++){
		QJsonValueRef ref = *it;
		if(ref.isObject()){
			Album album;
			MetaDataList v_md_tmp;
			ArtistList artists_tmp;

			if(parsePlaylist(artists_tmp, album, v_md_tmp, ref.toObject())){
				v_md << v_md_tmp;

				for(const Artist& artist_tmp : artists_tmp){
					if(!artists.contains(artist_tmp.id()) && artist_tmp.id() > 0){
						artists << artist_tmp;
					}
				}

				if(!albums.contains(album.id())){
					albums << album;
				}
			}
		}
	}

	return true;
}


bool SC::JsonParser::parsePlaylist(ArtistList& artists, Album& album, MetaDataList& v_md, QJsonObject object)
{
	Artist pl_artist;
	QString cover_download_url;

	AlbumId albumId;
	getInt("id", object, albumId);
	album.setId(albumId);

	QString album_name;
	getString("title", object, album_name);
	album.setName(album_name);

	getString("artwork_url", object, cover_download_url);
	album.setCoverDownloadUrls({cover_download_url});

	int num_songs;
	if(getInt("track_count", object, num_songs)){
		album.setSongcount(TrackNum(num_songs));
	}

	int length;
	if(getInt("duration", object, length)){
		album.setDurationSec(length / 1000);
	}

	QJsonObject artist_object;
	if(getObject("user", object, artist_object))
	{
		parseArtist(pl_artist, artist_object);
		if(!artists.contains(pl_artist.id()) && pl_artist.id() > 0){
			artists << pl_artist;
		}
	}

	QJsonArray track_array;
	if(getArray("tracks", object, track_array))
	{
		ArtistList tmp_artists;
		MetaDataList v_md_tmp;
		parseTrackList(tmp_artists, v_md_tmp, track_array);
		for(const Artist& tmp_artist : tmp_artists)
		{
			if(!artists.contains(tmp_artist.id())){
				artists << tmp_artist;
			}
		}

		for(const MetaData& md : v_md_tmp)
		{
			if(!v_md.contains(md.id())){
				v_md << md;
			}
		}
	}

	QString permalink, purchase_url;
	if(getString("permalink", object, permalink)){
		album.addCustomField(permalink, tr("Permalink Url"), createLink("Soundcloud", permalink));
	}

	if(getString("purchase_url", object, purchase_url)){
		album.addCustomField(purchase_url, tr("Purchase Url"), createLink(purchase_url, purchase_url));
	}

	album_name = album.name();

	for(int i=0; i<v_md.count(); i++)
	{
		MetaData& md = v_md[i];
		md.setTrackNumber(TrackNum(i+1));
		md.setAlbum(album.name());
		md.setAlbumId(album.id());

		if(md.artistId() != pl_artist.id() && pl_artist.id() > 0 && md.artistId() > 0)
		{
			md.setAlbum( md.album() + " (by " + pl_artist.name() + ")");
			album_name = album.name() + " (by " + pl_artist.name() + ")";
		}

		if(!album.coverDownloadUrls().isEmpty()){
			v_md[i].setCoverDownloadUrls(album.coverDownloadUrls());
		}
	}

	album.setName(album_name);

	QStringList lst;
	for(const Artist& artist : artists){
		lst << artist.name();
	}

	album.setArtists(lst);

	return (album.id() > 0);
}


QString SC::JsonParser::createLink(const QString& name, const QString& target)
{
	Settings* s = Settings::instance();
	bool dark = (s->get<Set::Player_Style>() == 0);
	return Util::createLink(name, dark, true, target);
}


bool SC::JsonParser::getString(const QString& key, const QJsonObject& object, QString& str)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isString()){
			str = ref.toString();
			str.replace("\\n", "<br />");
			str.replace("\\\"", "\"");
			str = str.trimmed();
			return true;
		}
	}

	return false;
}

bool SC::JsonParser::getInt(const QString& key, const QJsonObject& object, int& i)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isDouble()){
			i = ref.toInt();
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::getArray(const QString& key, const QJsonObject& object, QJsonArray& arr)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isArray()){
			arr = ref.toArray();
			return true;
		}
	}

	return false;
}

bool SC::JsonParser::getObject(const QString& key, const QJsonObject& object, QJsonObject& o)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isObject()){
			o = ref.toObject();
			return true;
		}
	}

	return false;
}
