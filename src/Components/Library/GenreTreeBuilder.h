/* GenreTreeBuilder.h, (Created on 01.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#ifndef SAYONARA_PLAYER_GENRETREEBUILDER_H
#define SAYONARA_PLAYER_GENRETREEBUILDER_H

#include "Utils/SetFwd.h"
#include "Utils/Tree.h"

class Genre;

namespace GenreTreeBuilder
{
	using GenreNode = Util::Tree<QString>;
	GenreNode* buildGenreDataTree(const Util::Set<Genre>& genres, bool createTree);
};

#endif //SAYONARA_PLAYER_GENRETREEBUILDER_H
