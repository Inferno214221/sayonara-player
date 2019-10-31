/* typedefs.h */

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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <cstdint>
#include "Utils/SetFwd.h"
#include "Utils/MetaData/MetaDataFwd.h"
#include <QMetaType>

class QByteArray;
class QString;
class QStringList;
template <typename A, typename B> struct QPair;
template <typename A, typename B> class QMap;
template <typename T> class QList;

/**
 * @brief Sayonara Typedefs
 * @ingroup Helper
 */
using StringPair=QPair<QString, QString>;
using IntList=QList<int>;
using IdList=QList<int>;
using IdxList=QList<int> ;
using BoolList=QList<bool> ;
using Id=int32_t;
using ArtistId=Id;
using AlbumId=Id;
using TrackID=Id;
using IntSet=Util::Set<int>;
using IndexSet=Util::Set<int>;
using IdSet=Util::Set<Id>;
using LibraryId=int8_t;
using DbId=uint8_t;
using Byte=uint8_t;
using Disc=uint8_t;
using TrackNum=uint16_t;
using Year=uint16_t;
using Seconds=int32_t;
using MilliSeconds=int64_t;
using MicroSeconds=int64_t;
using NanoSeconds=int64_t;
using Bitrate=uint32_t;
using Filesize=uint64_t;

template<typename K, typename V>
using PairList = QList<QPair<K,V>>;

enum class Rating : uint8_t
{
	Zero=0,
	One=1,
	Two=2,
	Three=3,
	Four=4,
	Five=5,
	Last=6
};


Q_DECLARE_METATYPE(Rating)

#endif
