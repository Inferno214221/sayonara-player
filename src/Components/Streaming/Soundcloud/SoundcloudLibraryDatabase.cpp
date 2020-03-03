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

SC::LibraryDatabase::LibraryDatabase(const QString& connectionName, DbId databaseId, LibraryId libraryId) :
	::DB::LibraryDatabase(connectionName, databaseId, libraryId)
{}

SC::LibraryDatabase::~LibraryDatabase() = default;

QString SC::LibraryDatabase::fetchQueryArtists(bool alsoEmpty) const
{
	QString sql =
		"SELECT "
		"artists.artistid				AS artistID, "
		"artists.name					AS artistName, "
		"artists.permalink_url			AS permalink_url, "
		"artists.description			AS description, "
		"artists.followers_following	AS followers_following, "
		"artists.cover_url				AS cover_url, "
		"artists.name					AS albumArtistName, "
		"COUNT(DISTINCT tracks.trackid)			AS trackCount, "
		"GROUP_CONCAT(DISTINCT albums.albumid)	AS artistAlbums "
		"FROM artists ";

	QString join = " INNER JOIN ";
	if(alsoEmpty){
		join = " LEFT OUTER JOIN ";
	}

	sql +=	join + " tracks ON artists.artistID = tracks.artistID " +
			join + " albums ON albums.albumID = tracks.albumID ";

	return sql;
}

QString SC::LibraryDatabase::fetchQueryAlbums(bool alsoEmpty) const
{
	QString sql =
		"SELECT "
		"albums.albumID				AS albumID, "
		"albums.name				AS albumName, "
		"SUM(tracks.length) / 1000	AS albumLength, "
		"albums.rating				AS albumRating, "
		"albums.permalink_url		AS permalink_url, "
		"albums.purchase_url		AS purchase_url, "
		"albums.cover_url			AS cover_url, "
		"COUNT(DISTINCT tracks.trackid)				AS trackCount, "
		"MAX(tracks.year)							AS albumYear, "
		"GROUP_CONCAT(DISTINCT artists.name)		AS albumArtists, "
		"GROUP_CONCAT(DISTINCT tracks.discnumber)	AS discnumbers "
		"FROM albums ";

	QString join = "INNER JOIN";
	if(alsoEmpty){
		join = "LEFT OUTER JOIN";
	}

	sql +=	join + " tracks ON albums.albumID = tracks.albumID " +
			join + " artists ON artists.artistID = tracks.artistID ";

	return sql;
}

QString SC::LibraryDatabase::fetchQueryTracks() const
{
	return	"SELECT "
			"tracks.trackID			AS trackID, "
			"tracks.title			AS trackTitle, "
			"tracks.length			AS trackLength, "
			"tracks.year			AS trackYear, "
			"tracks.bitrate			AS trackBitrate, "
			"tracks.filename		AS trackFilename, "
			"tracks.track			AS trackNum, "
			"albums.albumID			AS albumID, "
			"artists.artistID		AS artistID, "
			"albums.name			AS albumName, "
			"artists.name			AS artistName, "
			"tracks.genre			AS genrename, "
			"tracks.filesize		AS filesize, "
			"tracks.discnumber		AS discnumber, "
			"tracks.purchase_url	AS purchase_url, "
			"tracks.cover_url		AS cover_url, "
			"tracks.rating			AS rating "
			"FROM tracks "
			"INNER JOIN albums ON tracks.albumID = albums.albumID "
			"INNER JOIN artists ON tracks.artistID = artists.artistID ";
}

bool SC::LibraryDatabase::dbFetchTracks(Query& q, MetaDataList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Cannot fetch tracks from database");
		return false;
	}

	if(!q.last()) {
		return true;
	}

	for(bool isElement = q.first(); isElement; isElement = q.next())
	{
		MetaData data;

		data.setId(q.value(0).toInt());
		data.setTitle(q.value(1).toString());
		data.setDurationMs(q.value(2).toInt());
		data.setYear(q.value(3).value<Year>());
		data.setBitrate(q.value(4).value<Bitrate>());
		data.setFilepath(q.value(5).toString());
		data.setTrackNumber(q.value(6).value<TrackNum>());
		data.setAlbumId(q.value(7).toInt());
		data.setArtistId(q.value(8).toInt());
		data.setAlbum(q.value(9).toString().trimmed());
		data.setArtist(q.value(10).toString().trimmed());
		data.setGenres(q.value(11).toString().split(","));
		data.setFilesize(q.value(12).value<Filesize>());
		data.setDiscnumber(q.value(13).value<Disc>());
		data.addCustomField("purchase_url", Lang::get(Lang::PurchaseUrl), q.value(14).toString());
		data.setCoverDownloadUrls({q.value(15).toString()});
		data.setRating(q.value(16).value<Rating>());
		data.setDatabaseId(module()->databaseId());

		result << data;
	}

	return true;
}

bool SC::LibraryDatabase::dbFetchAlbums(Query& q, AlbumList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Could not get all albums from database");
		return false;
	}

	while(q.next())
	{
		Album album;

		album.setId(q.value(0).toInt());
		album.setName(q.value(1).toString().trimmed());
		album.setDurationSec(q.value(2).value<Seconds>());
		album.setRating(q.value(3).value<Rating>());
		album.addCustomField("permalink_url", "Permalink Url", q.value(4).toString());
		album.addCustomField("purchase_url", "Purchase Url", q.value(5).toString());
		album.setCoverDownloadUrls({q.value(6).toString()});
		album.setSongcount(q.value(7).value<TrackNum>());
		album.setYear(q.value(8).value<Year>());

		QStringList artistList = q.value(9).toString().split(',');
		album.setArtists(artistList);

		QStringList discnumberList = q.value(10).toString().split(',');
		auto discnumbers = album.discnumbers();
		discnumbers.clear();

		for(const QString& disc : discnumberList)
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

		album.setDiscnumbers(discnumbers);
		album.setDatabaseId(module()->databaseId());

		result << album;
	}

	return true;
}

bool SC::LibraryDatabase::dbFetchArtists(Query& q, ArtistList& result) const
{
	result.clear();

	if (!q.exec()) {
		q.showError("Could not get all artists from database");
		return false;
	}

	if(!q.last()){
		return true;
	}

	for(bool isElement = q.first(); isElement; isElement = q.next())
	{
		Artist artist;

		artist.setId(q.value(0).toInt());
		artist.setName(q.value(1).toString().trimmed());

		artist.addCustomField("permalink_url", "Permalink Url", q.value(2).toString());
		artist.addCustomField("description", "Description", q.value(3).toString());
		artist.addCustomField("followers_following", "Followers/Following", q.value(4).toString());

		artist.setCoverDownloadUrls({q.value(5).toString()});
		artist.setSongcount(q.value(7).value<uint16_t>());
		QStringList list = q.value(8).toString().split(',');
		artist.setAlbumcount(uint16_t(list.size()));
		artist.setDatabaseId(module()->databaseId());

		result << artist;
	}

	return true;
}

ArtistId SC::LibraryDatabase::updateArtist(const Artist& artist)
{
	QString cover_url;
	if(!artist.coverDownloadUrls().isEmpty()) {
		cover_url = artist.coverDownloadUrls().first();
	}

	Query q = this->update
	(
		"artists",
		{
			{"name",				artist.name()},
			{"cissearch",			artist.name().toLower()},
			{"permalink_url",		artist.customField("permalink_url")},
			{"description",			artist.customField("description")},
			{"followers_following", artist.customField("followers_following")},
			{"cover_url",			cover_url}
		},
		{"sc_id", artist.id()},
		QString("Soundcloud: Cannot update artist %1").arg(artist.name())
	);

	if(q.hasError()) {
		return -1;
	}

	return getArtistID(artist.name());
}

ArtistId SC::LibraryDatabase::insertArtistIntoDatabase (const QString& artist)
{
	Q_UNUSED(artist)
	return -1;
}

bool SC::LibraryDatabase::getAllAlbums(AlbumList& result, bool alsoEmpty) const
{
	Query q(module());

	QString query =
		fetchQueryAlbums(alsoEmpty) +
		" GROUP BY albums.albumID, albums.name, albums.rating "
	;

	q.prepare(query);

	return dbFetchAlbums(q, result);
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
	if(!artist.coverDownloadUrls().isEmpty()) {
		cover_url = artist.coverDownloadUrls().first();
	}

	Query q = this->insert
	(
		"artists",
		{
			{"artistID",			artist.id()},
			{"name",				artist.name()},
			{"cissearch",			artist.name().toLower()},
			{"permalink_url",		artist.customField("permalink_url")},
			{"description",			artist.customField("description")},
			{"followers_following", artist.customField("followers_following")},
			{"cover_url",			cover_url}
		},
		QString("Soundcloud: Cannot insert artist %1").arg(artist.name())
	);

	if (q.hasError()) {
		return -1;
	}

	return getArtistID(artist.name());
}

AlbumId SC::LibraryDatabase::updateAlbum(const Album& album)
{
	QString cover_url;
	if(!album.coverDownloadUrls().isEmpty()) {
		cover_url = album.coverDownloadUrls().first();
	}

	Query q = this->update
	(
		"albums",
		{
			{"name",			album.name()},
			{"cissearch",		album.name().toLower()},
			{"permalink_url",	album.customField("permalink_url")},
			{"purchase_url",	album.customField("purchase_url")},
			{"cover_url",		cover_url}
		},
		{"sc_id", album.id()},
		QString("Soundcloud: Cannot update album %1").arg(album.name())
	);

	if(q.hasError()) {
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
	if(!album.coverDownloadUrls().isEmpty()) {
		cover_url = album.coverDownloadUrls().first();
	}

	Query q = this->insert
	(
		"albums",
		{
			{"albumID",			album.id()},
			{"name",			album.name()},
			{"cissearch",		album.name().toLower()},
			{"permalink_url",	album.customField("permalink_url")},
			{"purchase_url",	album.customField("purchase_url")},
			{"cover_url",		cover_url}
		},
		QString("Soundcloud: Cannot insert album %1").arg(album.name())
	);

	if(q.hasError()) {
		return -1;
	}

	return getAlbumID(album.name());
}

bool SC::LibraryDatabase::updateTrack(const MetaData& md)
{
	QString cover_url;
	if(!md.coverDownloadUrls().isEmpty()) {
		cover_url = md.coverDownloadUrls().first();
	}

	Query q = this->update
	(
		"tracks",
		{
			{"title",			md.title()},
			{"filename",		md.filepath()},
			{"albumID",			md.albumId()},
			{"artistID",		md.artistId()},
			{"length",			QVariant::fromValue(md.durationMs())},
			{"year",			md.year()},
			{"track",			md.trackNumber()},
			{"bitrate",			md.bitrate()},
			{"genre",			md.genresToList().join(",")},
			{"filesize",		QVariant::fromValue(md.filesize())},
			{"discnumber",		md.discnumber()},
			{"cissearch",		md.title().toLower()},
			{"purchase_url",	md.customField("purchase_url")},
			{"cover_url",		cover_url},
		},
		{"trackID", md.id()},
		QString("Soundcloud: Cannot update track %1").arg(md.filepath())
	);

	return (q.hasError() == false);
}

bool SC::LibraryDatabase::insertTrackIntoDatabase(const MetaData& md, int artistId, int albumId, int album_artistId)
{
	Q_UNUSED(album_artistId)
	return insertTrackIntoDatabase(md, artistId, albumId);
}

bool SC::LibraryDatabase::insertTrackIntoDatabase(const MetaData& md, int artistId, int albumId)
{
	int new_id = getTrackById(md.id()).id();
	if(new_id > 0) {
		return updateTrack(md);
	}

	QString cover_url;
	if(!md.coverDownloadUrls().isEmpty()) {
		cover_url = md.coverDownloadUrls().first();
	}

	Query q = this->insert
	(
		"tracks",
		{
			{"trackID",			md.id()},
			{"title",			md.title()},
			{"filename",		md.filepath()},
			{"albumID",			albumId},
			{"artistID",		artistId},
			{"length",			QVariant::fromValue(md.durationMs())},
			{"year",			md.year()},
			{"track",			md.trackNumber()},
			{"bitrate",			md.bitrate()},
			{"genre",			md.genresToList().join(",")},
			{"filesize",		QVariant::fromValue(md.filesize())},
			{"discnumber",		md.discnumber()},
			{"cissearch",		md.title().toLower()},
			{"purchase_url",	md.customField("purchase_url")},
			{"cover_url",		cover_url},
		},
		QString("Soundcloud: Cannot insert track %1").arg(md.filepath())
	);

	return (q.hasError() == false);
}

bool SC::LibraryDatabase::storeMetadata(const MetaDataList& v_md)
{
	if(v_md.isEmpty()) {
		return true;
	}

	module()->db().transaction();

	for(const MetaData& md : v_md)
	{
		spLog(Log::Debug, this) << "Looking for " << md.artist() << " and " << md.album();
		if(md.albumId() == -1 || md.artistId() == -1)
		{
			spLog(Log::Warning, this) << "AlbumID = " << md.albumId() << " - ArtistID = " << md.artistId();
			continue;
		}

		insertTrackIntoDatabase (md, md.artistId(), md.albumId());
	}

	return module()->db().commit();
}

bool SC::LibraryDatabase::searchInformation(SC::SearchInformationList& search_information)
{
	Query q = this->runQuery
	(
		"SELECT artistId, albumId, trackId, allCissearch FROM track_search_view;",
		"Soundcloud: Cannot get search Information"
	);

	if(q.hasError()) {
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
