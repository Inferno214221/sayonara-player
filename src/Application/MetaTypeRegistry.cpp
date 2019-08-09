/* MetaTypeRegistry.cpp */

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



#include "MetaTypeRegistry.h"
#include <QObject>

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/typedefs.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include "Components/Covers/CoverLocation.h"

MetaTypeRegistry::MetaTypeRegistry(QObject* parent) :
	QObject(parent)
{
	qRegisterMetaType<StringPair>("StringPair");
	qRegisterMetaType<IntList>("IntList");
	qRegisterMetaType<IdList>("IDList");
	qRegisterMetaType<IdxList>("IdxList");
	//qRegisterMetaType<BoolList>("BoolList");
	qRegisterMetaType<ArtistId>("ArtistID");
	qRegisterMetaType<AlbumId>("AlbumID");
	qRegisterMetaType<TrackID>("TrackID");
	qRegisterMetaType<Id>("ID");
	qRegisterMetaType<IntSet>("IntSet");
	qRegisterMetaType<IndexSet>("IndexSet");
	qRegisterMetaType<IdSet>("IDSet");

	qRegisterMetaType<MetaData>("MetaData");
	qRegisterMetaType<MetaDataList>("MetaDataList");
	qRegisterMetaType<Album>("Album");
	qRegisterMetaType<AlbumList>("AlbumList");
	qRegisterMetaType<Artist>("Artist");
	qRegisterMetaType<ArtistList>("ArtistList");
	qRegisterMetaType<Genre>("Genre");
	qRegisterMetaType<Rating>("Rating");

	qRegisterMetaType<Log>("Log");
	qRegisterMetaType<Cover::Location>("Cover::Location");

	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<uint32_t>("uint32_t");

	qRegisterMetaType<int64_t>("int64_t");
	qRegisterMetaType<int32_t>("int32_t");

	qRegisterMetaType<Util::Set<int>>("Util::Set<int>");

	qRegisterMetaType<Seconds>("TimestampSec");
}

MetaTypeRegistry::~MetaTypeRegistry()
{}

