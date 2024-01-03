/* LibraryItemInfo.h */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_LIBRARYITEMINFO_H
#define SAYONARA_PLAYER_LIBRARYITEMINFO_H

#include "Utils/Pimpl.h"
#include "Utils/Set.h"
#include "Utils/typedefs.h"

#include <QList>
#include <QPair>

enum class InfoStrings :
	uint8_t
{
	TrackCount = 0,
	AlbumCount,
	ArtistCount,
	CreateDate,
	ModifyDate,
	Filesize,
	PlayingTime,
	Year,
	Sampler,
	Bitrate,
	Genre,
	Filetype,
	Comment
};

class QString;
namespace Cover
{
	class Location;
}
class MetaDataList;
class LibraryItemInfo
{
	PIMPL(LibraryItemInfo)

	public:
		using AdditionalInfo = QList<QPair<QString, QString>>;

		explicit LibraryItemInfo(const MetaDataList& metaDataList);
		virtual ~LibraryItemInfo();

		[[nodiscard]] virtual auto additionalData() const -> AdditionalInfo = 0;
		[[nodiscard]] virtual auto coverLocation() const -> Cover::Location = 0;
		[[nodiscard]] virtual auto header() const -> QString = 0;
		[[nodiscard]] virtual auto subheader() const -> QString = 0;

		[[nodiscard]] QStringList paths() const;
		[[nodiscard]] AdditionalInfo additionalInfo() const;

		[[nodiscard]] const Util::Set<QString>& albums() const;

		static QString convertInfoKeyToString(InfoStrings infoKey);

	protected:
		[[nodiscard]] const Util::Set<QString>& artists() const;
		[[nodiscard]] const Util::Set<QString>& albumArtists() const;

		[[nodiscard]] const Util::Set<AlbumId>& albumIds() const;
		[[nodiscard]] const Util::Set<ArtistId>& artistIds() const;

		[[nodiscard]] QString calcArtistString() const;
		[[nodiscard]] QString calcAlbumString() const;
};

#endif //SAYONARA_PLAYER_LIBRARYITEMINFO_H
