/* ConverterFactory.cpp */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "ConverterFactory.h"
#include "Components/Converter/Converter.h"
#include "Components/Converter/OggConverter.h"
#include "Components/Converter/LameConverter.h"
#include "Components/Converter/OpusConverter.h"
#include "Components/Playlist/Playlist.h"

#include "Interfaces/PlaylistInterface.h"

#include "Utils/MetaData/MetaDataList.h"

namespace
{
	void addTracks(PlaylistAccessor* playlistAccessor, Converter* converter)
	{
		auto pl = playlistAccessor->playlist(playlistAccessor->currentIndex());
		if(pl && converter)
		{
			converter->addMetadata(pl->tracks());
		}
	}

	bool checkConverter(Converter* converter)
	{
		return converter->isAvailable();
	}
}

struct ConverterFactory::Private
{
	PlaylistAccessor* playlistAccessor;

	Private(PlaylistAccessor* playlistAccessor) :
		playlistAccessor(playlistAccessor)
	{}
};

ConverterFactory::ConverterFactory(PlaylistAccessor* playlistAccessor)
{
	m = Pimpl::make<Private>(playlistAccessor);
}

ConverterFactory::~ConverterFactory() = default;

Converter* ConverterFactory::createOggConverter(int quality)
{
	return new OggConverter(quality, nullptr);
}

Converter* ConverterFactory::createLameConverter(Bitrate bitrate, int quality)
{
	return new LameConverter(bitrate == Bitrate::Constant, quality, nullptr);
}

Converter* ConverterFactory::createOpusConverter(Bitrate bitrate, int quality)
{
	return new OpusConverter(bitrate == Bitrate::Constant, quality, nullptr);
}

Converter* ConverterFactory::finalizeConverter(Converter* converter)
{
	if(checkConverter(converter)){
		addTracks(m->playlistAccessor, converter);
	}

	return converter;
}
