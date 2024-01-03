/* LocalSimilarArtistFetcher.cpp */
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
#include "LocalSimilarArtistFetcher.h"
#include "ArtistMatch.h"

#include "Utils/Compressor/Compressor.h"
#include "Utils/FileUtils.h"
#include "Utils/StandardPaths.h"
#include "Utils/Utils.h"

#include <QStringList>

using DynamicPlayback::LocalSimilarArtistFetcher;
using DynamicPlayback::ArtistMatch;

struct LocalSimilarArtistFetcher::Private
{
	ArtistMatch artistMatch;
};

LocalSimilarArtistFetcher::LocalSimilarArtistFetcher(const QString& artistName, QObject* parent) :
	SimilarArtistFetcher(artistName, parent)
{
	m = Pimpl::make<Private>();
}

LocalSimilarArtistFetcher::~LocalSimilarArtistFetcher() = default;

const ArtistMatch& LocalSimilarArtistFetcher::similarArtists() const
{
	return m->artistMatch;
}

void LocalSimilarArtistFetcher::fetchSimilarArtists(const QString& artistName)
{
	const auto similarArtistDir = Util::similarArtistsPath();
	const auto filename = QString("%1/%2.comp")
		.arg(similarArtistDir)
		.arg(artistName);

	if(Util::File::exists(filename))
	{
		QByteArray compressedData;
		const auto success = Util::File::readFileIntoByteArray(filename, compressedData);
		if(success)
		{
			const auto decompressedData = Compressor::decompress(compressedData);
			const auto decompressedString = QString::fromLocal8Bit(decompressedData);

			m->artistMatch = ArtistMatch::fromString(decompressedString);
		}
	}

	emit sigFinished();
}
