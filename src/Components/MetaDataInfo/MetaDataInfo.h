/* MetaDataInfo.h */

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

#ifndef METADATAINFO_H
#define METADATAINFO_H

#include "Utils/Pimpl.h"
#include "Components/Covers/CoverLocation.h"

#include <QObject>
#include <QMap>
#include <QList>

class Genre;
class LibraryDatabase;

/**
 * @brief The InfoStrings enum
 * @ingroup MetaDataHelper
 */
enum class InfoStrings : uint8_t
{
		nTracks = 0,        // set by MetaDataInfo
		nAlbums,        // set by ArtistInfo, AlbumInfo
		nArtists,        // set by ArtistInfo, AlbumInfo
		CreateDate,        // set by MetaDataInfo
		ModifyDate,        // set by MetaDataInfo
		Filesize,        // set by MetaDataInfo
		PlayingTime,    // set by MetaDataInfo
		Year,            // set by MetaDataInfo
		Sampler,        // set by AlbumInfo
		Bitrate,        // set by MetaDataInfo
		Genre,            // set by MetaDataInfo
		Filetype,        // set by MetaDataInfo
		Comment            // set by MetaDataInfo
};

/**
 * @brief The MetaDataInfo class
 * @ingroup MetaDataHelper
 */
class MetaDataInfo :
	public QObject
{
	PIMPL(MetaDataInfo)

	protected:
		QString mHeader;
		QString mSubheader;
		QMap<InfoStrings, QString> mInfo;
		QList<StringPair> mAdditionalInfo;

		QString calcTracknumString(TrackNum tracknum);
		QString calcArtistString() const;
		QString calcAlbumString();

		virtual void calcCoverLocation();
		virtual void calcSubheader();
		virtual void calcHeader();

		void insertPlayingTime(MilliSeconds ms);
		void insertGenre(const Util::Set<Genre>& genres);
		void insertFilesize(uint64_t filesize);
		void insertFiletype(const Util::Set<QString>& filetypes);
		void insertComment(const Util::Set<QString>& comments);
		void insertCreatedates(uint64_t minDate, uint64_t maxDate);
		void insertModifydates(uint64_t minDate, uint64_t maxDate);

		void insertIntervalInfoField(InfoStrings key, int min, int max);
		void insertNumericInfoField(InfoStrings key, int number);

	public:
		explicit MetaDataInfo(const MetaDataList& tracks);
		virtual ~MetaDataInfo();

		virtual QString header() const;
		virtual QString subheader() const;
		virtual QString infostring() const;
		virtual QList<StringPair> infostringMap() const;

		virtual Cover::Location coverLocation() const;

		const Util::Set<QString>& albums() const;
		const Util::Set<QString>& artists() const;
		const Util::Set<QString>& albumArtists() const;

		const Util::Set<AlbumId>& albumIds() const;
		const Util::Set<ArtistId>& artistIds() const;
		const Util::Set<ArtistId>& albumArtistIds() const;

		QStringList paths() const;

	private:
		void calcCoverLocation(const MetaDataList& tracks);
		void calcSubheader(quint16 tracknum);
		void calcHeader(const MetaDataList& tracks);

		QString getInfoString(InfoStrings idx) const;
};

#endif // METADATAINFO_H
