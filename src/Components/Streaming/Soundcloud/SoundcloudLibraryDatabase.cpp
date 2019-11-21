#include "SoundcloudLibraryDatabase.h"
#include "SearchInformation.h"

#include "Database/Query.h"

#include "Utils/typedefs.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

using ::DB::Query;

SC::LibraryDatabase::LibraryDatabase(const QString& connection_name, DbId db_id, LibraryId library_id) :
	::DB::LibraryDatabase(connection_name, db_id, library_id)
{}

SC::LibraryDatabase::~LibraryDatabase() = default;

QString SC::LibraryDatabase::fetch_query_artists(bool also_empty) const
{
	QString sql =
			"SELECT "
			"artists.artistid AS artistID, "
			"artists.name AS artistName, "
			"artists.permalink_url AS permalink_url, "
			"artists.description AS description, "
			"artists.followers_following AS followers_following, "
			"artists.cover_url AS cover_url, "
			"artists.name AS albumArtistName, "
			"COUNT(DISTINCT tracks.trackid) AS trackCount, "
			"GROUP_CONCAT(DISTINCT albums.albumid) AS artistAlbums "
			"FROM artists ";

	QString join = "INNER JOIN";
	if(also_empty){
		join = "LEFT OUTER JOIN";
	}

	sql +=	join + " tracks ON artists.artistID = tracks.artistID " +
			join + " albums ON albums.albumID = tracks.albumID ";

	return sql;
}

QString SC::LibraryDatabase::fetch_query_albums(bool also_empty) const
{
	QString sql =
			"SELECT "
			"albums.albumID AS albumID, "
			"albums.name AS albumName, "
			"SUM(tracks.length) / 1000 AS albumLength, "
			"albums.rating AS albumRating, "
			"albums.permalink_url AS permalink_url, "
			"albums.purchase_url AS purchase_url, "
			"albums.cover_url AS cover_url, "
			"COUNT(DISTINCT tracks.trackid) AS trackCount, "
			"MAX(tracks.year) AS albumYear, "
			"GROUP_CONCAT(DISTINCT artists.name) AS albumArtists, "
			"GROUP_CONCAT(DISTINCT tracks.discnumber) AS discnumbers "
			"FROM albums ";

	QString join = "INNER JOIN";
	if(also_empty){
		join = "LEFT OUTER JOIN";
	}

	sql +=	join + " tracks ON albums.albumID = tracks.albumID " +
			join + " artists ON artists.artistID = tracks.artistID ";

	return sql;
}

QString SC::LibraryDatabase::fetch_query_tracks() const
{
	return	"SELECT "
			"tracks.trackID AS trackID, "
			"tracks.title AS trackTitle, "
			"tracks.length AS trackLength, "
			"tracks.year AS trackYear, "
			"tracks.bitrate AS trackBitrate, "
			"tracks.filename AS trackFilename, "
			"tracks.track AS trackNum, "
			"albums.albumID AS albumID, "
			"artists.artistID AS artistID, "
			"albums.name AS albumName, "
			"artists.name AS artistName, "
			"tracks.genre AS genrename, "
			"tracks.filesize AS filesize, "
			"tracks.discnumber AS discnumber, "
			"tracks.purchase_url AS purchase_url, "
			"tracks.cover_url AS cover_url, "
			"tracks.rating AS rating "
			"FROM tracks "
			"INNER JOIN albums ON tracks.albumID = albums.albumID "
			"INNER JOIN artists ON tracks.artistID = artists.artistID ";
}

bool SC::LibraryDatabase::db_fetch_tracks(Query& q, MetaDataList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Cannot fetch tracks from database");
		return false;
	}

	if(!q.last()){
		return true;
	}

	for(bool is_element = q.first(); is_element; is_element = q.next())
	{
		MetaData data;

		data.set_id(			q.value(0).toInt());
		data.set_title(			q.value(1).toString());
		data.set_duration_ms(	q.value(2).toInt());
		data.set_year(			q.value(3).value<Year>());
		data.set_bitrate(		q.value(4).value<Bitrate>());
		data.set_filepath(		q.value(5).toString());
		data.set_track_number(	q.value(6).value<TrackNum>());
		data.set_album_id(		q.value(7).toInt());
		data.set_artist_id(		q.value(8).toInt());
		data.set_album(			q.value(9).toString().trimmed());
		data.set_artist(		q.value(10).toString().trimmed());
		data.set_genres(		q.value(11).toString().split(","));
		data.set_filesize(		q.value(12).value<Filesize>());
		data.set_discnumber(	q.value(13).value<Disc>());
		data.add_custom_field("purchase_url", Lang::get(Lang::PurchaseUrl), q.value(14).toString());
		data.set_cover_download_urls({q.value(15).toString()});
		data.set_rating(		q.value(16).value<Rating>());
		data.set_db_id(module()->db_id());

		result << data;
	}

	return true;
}

bool SC::LibraryDatabase::db_fetch_albums(Query& q, AlbumList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all albums from database");
		return false;
	}

	while(q.next())
	{
		Album album;

		album.set_id(q.value(0).toInt());
		album.set_name(q.value(1).toString().trimmed());
		album.set_duration_sec(q.value(2).value<Seconds>());
		album.set_rating(q.value(3).value<Rating>());
		album.add_custom_field("permalink_url", "Permalink Url", q.value(4).toString());
		album.add_custom_field("purchase_url", "Purchase Url", q.value(5).toString());
		album.set_cover_download_urls({q.value(6).toString()});
		album.set_songcount(q.value(7).value<TrackNum>());
		album.set_year(q.value(8).value<Year>());

		QStringList lst_artists =	q.value(9).toString().split(',');
		album.set_artists(lst_artists);

		QStringList lst_discnumbers = q.value(10).toString().split(',');
		auto discnumbers = album.discnumbers();
		discnumbers.clear();

		for(const QString& disc : lst_discnumbers)
		{
			auto d = Disc(disc.toInt());
			if(discnumbers.contains(d)) {
				continue;
			}

			discnumbers << d;
		}

		if(discnumbers.isEmpty()) {
			discnumbers << 1;
		}

		album.set_discnumbers(discnumbers);
		album.set_db_id(module()->db_id());

		result << album;
	};

	return true;
}

bool SC::LibraryDatabase::db_fetch_artists(Query& q, ArtistList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all artists from database");
		return false;
	}

	if(!q.last()){
		return true;
	}

	for(bool is_element=q.first(); is_element; is_element = q.next())
	{
		Artist artist;

		artist.set_id(			q.value(0).toInt());
		artist.set_name(		q.value(1).toString().trimmed());

		artist.add_custom_field("permalink_url", "Permalink Url", q.value(2).toString());
		artist.add_custom_field("description", "Description", q.value(3).toString());
		artist.add_custom_field("followers_following", "Followers/Following", q.value(4).toString());

		artist.set_cover_download_urls({q.value(5).toString()});
		artist.set_songcount(			q.value(7).value<uint16_t>());
		QStringList list =				q.value(8).toString().split(',');
		artist.set_albumcount(			uint16_t(list.size()));
		artist.set_db_id(module()->db_id());

		result << artist;
	}

	return true;
}

ArtistId SC::LibraryDatabase::updateArtist(const Artist& artist)
{
	QString cover_url;
	if(!artist.cover_download_urls().isEmpty()) {
		cover_url = artist.cover_download_urls().first();
	}

	Query q = this->update
	(
		"artists",
		{
			{"name",				artist.name()},
			{"cissearch",			artist.name().toLower()},
			{"permalink_url",		artist.get_custom_field("permalink_url")},
			{"description",			artist.get_custom_field("description")},
			{"followers_following", artist.get_custom_field("followers_following")},
			{"cover_url",			cover_url}
		},
		{"sc_id", artist.id()},
		QString("Soundcloud: Cannot update artist %1").arg(artist.name())
	);

	if(q.has_error()) {
		return -1;
	}

	return getArtistID(artist.name());
}

ArtistId SC::LibraryDatabase::insertArtistIntoDatabase (const QString& artist)
{
	Q_UNUSED(artist)
	return -1;
}

ArtistId SC::LibraryDatabase::insertArtistIntoDatabase (const Artist& artist)
{
	Artist tmp_artist;
	if(getArtistByID(artist.id(), tmp_artist))
	{
		if(tmp_artist.id() > 0) {
			return updateArtist(artist);
		}
	}

	QString cover_url;
	if(!artist.cover_download_urls().isEmpty()) {
		cover_url = artist.cover_download_urls().first();
	}

	Query q = this->insert
	(
		"artists",
		{
			{"artistID",			artist.id()},
			{"name",				artist.name()},
			{"cissearch",			artist.name().toLower()},
			{"permalink_url",		artist.get_custom_field("permalink_url")},
			{"description",			artist.get_custom_field("description")},
			{"followers_following", artist.get_custom_field("followers_following")},
			{"cover_url",			cover_url}
		},
		QString("Soundcloud: Cannot insert artist %1").arg(artist.name())
	);

	if (q.has_error()) {
		return -1;
	}

	return getArtistID(artist.name());
}


AlbumId SC::LibraryDatabase::updateAlbum(const Album& album)
{
	QString cover_url;
	if(!album.cover_download_urls().isEmpty()) {
		cover_url = album.cover_download_urls().first();
	}

	Query q = this->update
	(
		"albums",
		{
			{"name",			album.name()},
			{"cissearch",		album.name().toLower()},
			{"permalink_url",	album.get_custom_field("permalink_url")},
			{"purchase_url",	album.get_custom_field("purchase_url")},
			{"cover_url",		cover_url}
		},
		{"sc_id", album.id()},
		QString("Soundcloud: Cannot update album %1").arg(album.name())
	);

	if(q.has_error()) {
		return -1;
	}

	return getAlbumID(album.name());
}


AlbumId SC::LibraryDatabase::insertAlbumIntoDatabase (const QString& album)
{
	Q_UNUSED(album)
	return -1;
}

AlbumId SC::LibraryDatabase::insertAlbumIntoDatabase (const Album& album)
{
	QString cover_url;
	if(!album.cover_download_urls().isEmpty()) {
		cover_url = album.cover_download_urls().first();
	}

	Query q = this->insert
	(
		"albums",
		{
			{"albumID",			album.id()},
			{"name",			album.name()},
			{"cissearch",		album.name().toLower()},
			{"permalink_url",	album.get_custom_field("permalink_url")},
			{"purchase_url",	album.get_custom_field("purchase_url")},
			{"cover_url",		cover_url}
		},
		QString("Soundcloud: Cannot insert album %1").arg(album.name())
	);

	if(q.has_error()) {
		return -1;
	}

	return getAlbumID(album.name());
}

bool SC::LibraryDatabase::updateTrack(const MetaData& md)
{
	QString cover_url;
	if(!md.cover_download_urls().isEmpty()) {
		cover_url = md.cover_download_urls().first();
	}

	Query q = this->update
	(
		"tracks",
		{
			{"title",			md.title()},
			{"filename",		md.filepath()},
			{"albumID",			md.album_id()},
			{"artistID",		md.artist_id()},
			{"length",			QVariant::fromValue(md.duration_ms())},
			{"year",			md.year()},
			{"track",			md.track_number()},
			{"bitrate",			md.bitrate()},
			{"genre",			md.genres_to_list().join(",")},
			{"filesize",		QVariant::fromValue(md.filesize())},
			{"discnumber",		md.discnumber()},
			{"cissearch",		md.title().toLower()},
			{"purchase_url",	md.get_custom_field("purchase_url")},
			{"cover_url",		cover_url},
		},
		{"trackID", md.id()},
		QString("Soundcloud: Cannot update track %1").arg(md.filepath())
	);

	return (q.has_error() == false);
}

bool SC::LibraryDatabase::insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id, int album_artist_id)
{
	Q_UNUSED(album_artist_id)
	return insertTrackIntoDatabase(md, artist_id, album_id);
}

bool SC::LibraryDatabase::insertTrackIntoDatabase(const MetaData& md, int artist_id, int album_id)
{
	int new_id = getTrackById(md.id()).id();
	if(new_id > 0) {
		return updateTrack(md);
	}

	QString cover_url;
	if(!md.cover_download_urls().isEmpty()) {
		cover_url = md.cover_download_urls().first();
	}

	Query q = this->insert
	(
		"tracks",
		{
			{"trackID",			md.id()},
			{"title",			md.title()},
			{"filename",		md.filepath()},
			{"albumID",			album_id},
			{"artistID",		artist_id},
			{"length",			QVariant::fromValue(md.duration_ms())},
			{"year",			md.year()},
			{"track",			md.track_number()},
			{"bitrate",			md.bitrate()},
			{"genre",			md.genres_to_list().join(",")},
			{"filesize",		QVariant::fromValue(md.filesize())},
			{"discnumber",		md.discnumber()},
			{"cissearch",		md.title().toLower()},
			{"purchase_url",	md.get_custom_field("purchase_url")},
			{"cover_url",		cover_url},
		},
		QString("Soundcloud: Cannot insert track %1").arg(md.filepath())
	);

	return (q.has_error() == false);
}

bool SC::LibraryDatabase::store_metadata(const MetaDataList& v_md)
{
	if(v_md.isEmpty()) {
		return true;
	}

	module()->db().transaction();

	for(const MetaData& md : v_md)
	{
		sp_log(Log::Debug, this) << "Looking for " << md.artist() << " and " << md.album();
		if(md.album_id() == -1 || md.artist_id() == -1)
		{
			sp_log(Log::Warning, this) << "AlbumID = " << md.album_id() << " - ArtistID = " << md.artist_id();
			continue;
		}

		insertTrackIntoDatabase (md, md.artist_id(), md.album_id());
	}

	return module()->db().commit();
}

bool SC::LibraryDatabase::search_information(SC::SearchInformationList& search_information)
{
	Query q = this->run_query
	(
		"SELECT artistId, albumId, trackId, allCissearch FROM track_search_view;",
		"Soundcloud: Cannot get search Information"
	);

	if(q.has_error()) {
		return false;
	}

	while(q.next())
	{
		SC::SearchInformation info
		(
			q.value(0).toInt(),
			q.value(1).toInt(),
			q.value(2).toInt(),
			q.value(3).toString()
		);

		search_information << info;
	}

	return true;
}
