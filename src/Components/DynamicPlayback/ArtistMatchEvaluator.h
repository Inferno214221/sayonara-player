/* ArtistMatchEvaluator.h */
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
#ifndef SAYONARA_PLAYER_ARTISTMATCHEVALUATOR_H
#define SAYONARA_PLAYER_ARTISTMATCHEVALUATOR_H

#include "Utils/typedefs.h"
#include <QList>

namespace DynamicPlayback
{
	class ArtistMatch;

	/**
	 * @brief fetch artistsIds available in specific Library
	 * @param artistMatch similar artists
	 * @param libraryId library to search in, -1 for all libraries
	 * @return sorted list of artistsIds by random quality
	 */
	QList<ArtistId> evaluateArtistMatch(const ArtistMatch& artistMatch, LibraryId libraryId);
}
#endif //SAYONARA_PLAYER_ARTISTMATCHEVALUATOR_H
