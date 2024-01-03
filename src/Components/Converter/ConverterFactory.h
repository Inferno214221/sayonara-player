/* ConverterFactory.h */
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
#ifndef SAYONARA_PLAYER_CONVERTERFACTORY_H
#define SAYONARA_PLAYER_CONVERTERFACTORY_H

#include "Utils/Pimpl.h"

class PlaylistAccessor;
class Converter;
class ConverterFactory
{
	PIMPL(ConverterFactory)

	public:
		enum class ConvertType :
			uint8_t
		{
			OggVorbis = 0,
			OggOpus,
			Lame
		};

		enum class Bitrate :
			uint8_t
		{
			Constant = 0,
			Variable
		};

		ConverterFactory(PlaylistAccessor* playlistAccessor);
		~ConverterFactory();

		template<ConvertType t, typename...Args>
		typename std::enable_if<t == ConvertType::OggVorbis, Converter*>::type
		createConverter(Args&& ...args)
		{
			return finalizeConverter(createOggConverter(args...));
		}

		template<ConvertType t, typename...Args>
		typename std::enable_if<t == ConvertType::Lame || t == ConvertType::OggOpus, Converter*>::type
		createConverter(Args&& ...args)
		{
			if(t == ConvertType::Lame)
			{
				return finalizeConverter(createLameConverter(args...));
			}

			else if(t == ConvertType::OggOpus)
			{
				return finalizeConverter(createOpusConverter(args...));
			}

			else
			{
				return nullptr;
			}
		}

	private:
		Converter* createOggConverter(int quality);
		Converter* createLameConverter(Bitrate cbr, int quality);
		Converter* createOpusConverter(Bitrate cbr, int quality);

		Converter* finalizeConverter(Converter* converter);
};

#endif //SAYONARA_PLAYER_CONVERTERFACTORY_H
