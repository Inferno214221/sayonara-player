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

#include "SoundcloudJsonParser.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"
#include "Utils/StandardPaths.h"

#include <QJsonDocument>
#include <QDateTime>

#include <cassert>

using SC::JsonParser;

namespace
{
	using TrackArtistPair = std::pair<MetaData, Artist>;
	using AlbumTracksPair = std::pair<Album, QList<TrackArtistPair>>;

	enum class SCJsonItemType :
		uint8_t
	{
			Track = 0,
			Artist,
			Playlist
	};

	QString createLink(const QString& name, const QString& target)
	{
		auto* settings = Settings::instance();
		const auto dark = (settings->get<Set::Player_Style>() == 0);

		return Util::createLink(name, dark, true, target);
	}

	std::optional<QString> getString(const QString& key, const QJsonObject& object)
	{
		const auto it = object.find(key);
		if(it != object.end() && it->isString())
		{
			auto str = it->toString();
			str.replace("\\n", "<br />");
			str.replace("\\\"", "\"");
			str = str.trimmed();

			return std::optional {str};
		}

		return std::nullopt;
	}

	std::optional<int> getInt(const QString& key, const QJsonObject& object)
	{
		const auto it = object.find(key);
		return (it != object.end() && it->isDouble())
		       ? std::optional {it->toInt()}
		       : std::nullopt;
	}

	std::optional<QJsonArray> getArray(const QString& key, const QJsonObject& object)
	{
		const auto it = object.find(key);
		return (it != object.end() && it->isArray())
		       ? std::optional {it->toArray()}
		       : std::nullopt;
	}

	std::optional<QJsonObject> getObject(const QString& key, const QJsonObject& object)
	{
		const auto it = object.find(key);
		return (it != object.end() && it->isObject())
		       ? std::optional(it->toObject())
		       : std::nullopt;
	}

	std::optional<Artist> parseArtist(const QJsonObject& object)
	{
		auto artist = Artist {};

		if(const auto artistId = getInt("id", object); artistId.has_value())
		{
			artist.setId(artistId.value());
		}

		if(const auto artistName = getString("username", object); artistName.has_value())
		{
			artist.setName(artistName.value());
		}

		if(const auto coverDownloadUrl = getString("avatar_url", object); coverDownloadUrl.has_value())
		{
			artist.setCoverDownloadUrls({coverDownloadUrl.value()});
		}

		if(const auto website = getString("website", object); website.has_value())
		{
			artist.addCustomField("website", QObject::tr("Website"), website.value());
		}

		if(const auto permalink = getString("permalink", object); permalink.has_value())
		{
			artist.addCustomField("permalink", QObject::tr("Permalink Url"), permalink.value());
		}

		if(const auto description = getString("description", object); description.has_value())
		{
			artist.addCustomField("description", Lang::get(Lang::About), description.value());
		}

		const auto followers = getInt("followers_count", object);
		const auto following = getInt("followings_count", object);
		if(followers.has_value() && following.has_value())
		{
			artist.addCustomField("followers_following",
			                      QObject::tr("Followers/Following"),
			                      QString("%1/%2").arg(followers.value()).arg(following.value()));
		}

		return (artist.id() > 0)
		       ? std::optional {artist}
		       : std::nullopt;
	}

	ArtistList parseArtistArray(const QJsonArray& arr)
	{
		auto artists = ArtistList {};

		for(auto it = arr.begin(); it != arr.end(); it++)
		{
			if(it->isObject())
			{
				if(const auto artist = parseArtist(it->toObject()); artist.has_value())
				{
					assert(artist.value().id() > 0);
					artists << artist.value();
				}
			}
		}

		return artists;
	}

	std::optional<TrackArtistPair> parseTrack(const QJsonObject& object)
	{
		auto track = MetaData {};
		auto artist = Artist {};

		track.setAlbumId(0);

		if(const auto trackId = getInt("id", object); trackId.has_value())
		{
			track.setId(trackId.value());
		}

		if(const auto coverDownloadUrl = getString("artwork_url", object); coverDownloadUrl.has_value())
		{
			auto coverUrls = track.coverDownloadUrls();
			coverUrls.prepend(coverDownloadUrl.value());
			track.setCoverDownloadUrls(coverUrls);
		}

		if(const auto duration = getInt("duration", object); duration.has_value())
		{
			track.setDurationMs(static_cast<MilliSeconds>(duration.value()));
		}

		if(const auto year = getInt("release_year", object); year.has_value())
		{
			track.setYear(static_cast<Year>(year.value()));
		}

		if(const auto filesize = getInt("original_content_size", object); filesize.has_value())
		{
			track.setFilesize(static_cast<Filesize>(filesize.value()));
		}

		if(const auto title = getString("title", object); title.has_value())
		{
			track.setTitle(title.value());
		}

		if(const auto streamUrl = getString("stream_url", object); streamUrl.has_value())
		{
			track.setFilepath(streamUrl.value());
		}

		if(const auto genre = getString("genre", object); genre.has_value())
		{
			track.addGenre(Genre {genre.value()});
		}

		if(const auto purchaseUrl = getString("purchase_url", object); purchaseUrl.has_value())
		{
			track.addCustomField("purchase_url",
			                     QObject::tr("Purchase Url"),
			                     createLink(purchaseUrl.value(), purchaseUrl.value()));
		}

		if(const auto artistObject = getObject("user", object); artistObject.has_value())
		{
			if(const auto artistInfo = parseArtist(artistObject.value()); artistInfo.has_value())
			{
				artist = artistInfo.value();
				track.setArtist(artist.name());
				track.setArtistId(artist.id());

				const auto coverUrls = track.coverDownloadUrls() << artistInfo->coverDownloadUrls();
				track.setCoverDownloadUrls(coverUrls);
			}
		}

		if(const auto modified = getString("last_modified", object); modified.has_value())
		{
			const auto dateTime = QDateTime::fromString(modified.value(), Qt::DateFormat::ISODate);
			track.setModifiedDate(Util::dateToInt(dateTime));
		}

		if(const auto created = getString("created_at", object); created.has_value())
		{
			const auto dateTime = QDateTime::fromString(created.value(), Qt::DateFormat::ISODate);

			track.setCreatedDate(Util::dateToInt(dateTime));
			if(track.year() <= 0)
			{
				track.setYear(dateTime.date().year());
			}
		}

		if(const auto description = getString("description", object); description.has_value())
		{
			track.setComment(description.value());
		}

		return (track.filepath().size() > 0) && (track.id() > 0)
		       ? std::optional {std::make_pair(track, artist)}
		       : std::nullopt;
	}

	QList<TrackArtistPair> parseTrackArray(const QJsonArray& arr)
	{
		auto result = QList<TrackArtistPair> {};

		for(auto it = arr.begin(); it != arr.end(); it++)
		{
			if(it->isObject())
			{
				if(const auto trackArtistPair = parseTrack(it->toObject()); trackArtistPair.has_value())
				{
					auto track = trackArtistPair.value().first;
					auto artist = trackArtistPair.value().second;

					assert(track.id() > 0);
					track.setTrackNumber(static_cast<TrackNum>(result.size() + 1));

					result << std::make_pair(std::move(track), std::move(artist));
				}
			}
		}

		return result;
	}

	std::optional<AlbumTracksPair> parsePlaylist(const QJsonObject& object)
	{
		auto album = Album {};
		auto trackArtistPairs = QList<TrackArtistPair> {};
		auto albumArtist = Artist {};

		if(const auto albumId = getInt("id", object); albumId.has_value())
		{
			album.setId(albumId.value());
		}

		if(const auto albumName = getString("title", object); albumName.has_value())
		{
			album.setName(albumName.value());
		}

		if(const auto coverDownloadUrl = getString("artwork_url", object); coverDownloadUrl.has_value())
		{
			album.setCoverDownloadUrls({coverDownloadUrl.value()});
		}

		if(const auto trackCount = getInt("track_count", object); trackCount.has_value())
		{
			album.setSongcount(static_cast<TrackNum>(trackCount.value()));
		}

		if(const auto duration = getInt("duration", object); duration.has_value())
		{
			album.setDurationSec(duration.value() / 1000);
		}

		if(const auto permalink = getString("permalink", object); permalink.has_value())
		{
			album.addCustomField(permalink.value(),
			                     QObject::tr("Permalink Url"),
			                     createLink("Soundcloud", permalink.value()));
		}

		if(const auto purchaseUrl = getString("purchase_url", object); purchaseUrl.has_value())
		{
			const auto& url = purchaseUrl.value();
			album.addCustomField(url,
			                     QObject::tr("Purchase Url"),
			                     createLink(url, url));
		}

		if(const auto artistObject = getObject("user", object); artistObject.has_value())
		{
			if(const auto artist = parseArtist(artistObject.value()); artist.has_value())
			{
				albumArtist = artist.value();
				album.setAlbumArtist(albumArtist.name());
			}
		}

		if(const auto trackArray = getArray("tracks", object); trackArray.has_value())
		{
			trackArtistPairs = parseTrackArray(trackArray.value());

			auto artistNames = Util::Set<QString> {};

			for(auto& trackArtistPair : trackArtistPairs)
			{
				auto& track = trackArtistPair.first;
				track.setAlbumArtist(albumArtist.name(), albumArtist.id());

				track.setAlbumId(album.id());
				track.setAlbum(album.name());

				if(track.artistId() <= 0)
				{
					track.setArtistId(albumArtist.id());
					track.setArtist(albumArtist.name());
				}

				if(!album.coverDownloadUrls().isEmpty())
				{
					track.setCoverDownloadUrls(album.coverDownloadUrls());
				}

				artistNames.insert(track.artist());
			}

			album.setArtists(artistNames.toList());
		}

		return ((album.id() > 0) && !trackArtistPairs.isEmpty())
		       ? std::optional {std::make_pair(std::move(album), std::move(trackArtistPairs))}
		       : std::nullopt;
	}

	QList<AlbumTracksPair> parsePlaylistArray(const QJsonArray& arr)
	{
		auto result = QList<AlbumTracksPair> {};
		for(auto it = arr.begin(); it != arr.end(); it++)
		{
			if(it->isObject())
			{
				if(const auto albumTrackPair = parsePlaylist(it->toObject()); albumTrackPair.has_value())
				{
					const auto contains = Util::Algorithm::contains(result, [&](const auto& element) {
						return (element.first.id() == albumTrackPair.value().first.id());
					});

					if(!contains)
					{
						result.push_back(albumTrackPair.value());
					}
				}
			}
		}

		return result;
	}
}

struct JsonParser::Private
{
	QJsonDocument jsonDocument;
	QByteArray content;
	QJsonParseError error; // NOLINT

	explicit Private(const QByteArray& content) :
		content(content)
	{
		jsonDocument = QJsonDocument::fromJson(content, &error);
	}
};

JsonParser::JsonParser(const QByteArray& content)
{
	m = Pimpl::make<Private>(content);
	const auto targetFile = Util::tempPath("soundcloud.json");

	Util::File::writeFile(
		m->jsonDocument.toJson(QJsonDocument::Indented), targetFile
	);

	const auto parserError = m->error.error;
	if(parserError != QJsonParseError::NoError)
	{
		spLog(Log::Warning, this) << "Cannot parse json document: " << m->error.errorString();
	}
}

JsonParser::~JsonParser() = default;

bool JsonParser::parseArtists(ArtistList& artists) const
{
	artists.clear();

	if(m->jsonDocument.isArray())
	{
		artists = parseArtistArray(m->jsonDocument.array());
	}

	else if(m->jsonDocument.isObject())
	{
		if(const auto artist = parseArtist(m->jsonDocument.object()); artist.has_value())
		{
			assert(artist.value().id() > 0);
			artists << artist.value();
		}
	}

	return (!artists.empty());
}

bool JsonParser::parseTracks(ArtistList& artists, MetaDataList& tracks) const
{
	artists.clear();
	tracks.clear();

	if(!m->jsonDocument.isArray())
	{
		return false;
	}

	auto trackArtistPairs = parseTrackArray(m->jsonDocument.array());
	for(auto& trackArtistPair : trackArtistPairs)
	{
		auto& track = trackArtistPair.first;
		auto& artist = trackArtistPair.second;
		assert(track.id() > 0);

		if(!artists.contains(artist.id()))
		{
			artists.push_back(std::move(artist));
		}

		tracks << std::move(track);
	}

	return (!tracks.isEmpty());
}

bool JsonParser::parsePlaylists(ArtistList& artists, AlbumList& albums, MetaDataList& tracks) const
{
	artists.clear();
	albums.clear();
	tracks.clear();

	auto albumTrackArray = QList<AlbumTracksPair> {};

	if(m->jsonDocument.isArray())
	{
		albumTrackArray = parsePlaylistArray(m->jsonDocument.array());
	}

	else if(m->jsonDocument.isObject())
	{
		if(const auto albumTracksPair = parsePlaylist(m->jsonDocument.object()); albumTracksPair.has_value())
		{
			albumTrackArray << albumTracksPair.value();
		}
	}

	for(auto& albumTracksPair : albumTrackArray)
	{
		albums << albumTracksPair.first;

		for(auto& trackArtistPair : albumTracksPair.second)
		{
			auto& track = trackArtistPair.first;
			auto& artist = trackArtistPair.second;

			tracks << std::move(track);
			if(!artists.contains(artist.id()))
			{
				artists.push_back(std::move(artist));
			}
		}
	}

	return false;
}

std::optional<SC::OAuthTokenInfo> JsonParser::parseToken() const
{
	auto tokenInfo = SC::OAuthTokenInfo {};
	if(m->jsonDocument.isObject())
	{
		const auto object = m->jsonDocument.object();
		if(const auto token = getString("access_token", object); token.has_value())
		{
			tokenInfo.oauthToken = token.value();
		}

		if(const auto refreshToken = getString("refresh_token", object); refreshToken.has_value())
		{
			tokenInfo.refreshToken = refreshToken.value();
		}

		if(const auto expiresIn = getInt("expires_in", object); expiresIn.has_value())
		{
			tokenInfo.expiresIn = expiresIn.value();
		}

		return std::optional {tokenInfo};
	}

	return std::nullopt;
}
