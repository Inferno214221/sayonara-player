/* ContextMenuConfigurator.h, (Created on 12.01.2024) */

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
#ifndef SAYONARA_PLAYER_CONTEXTMENUCONFIGURATOR_H
#define SAYONARA_PLAYER_CONTEXTMENUCONFIGURATOR_H

#include "Utils/typedefs.h"

namespace Playlist
{
	class ContextMenu;
	class Model;

	uint64_t calcContextMenuEntries(ContextMenu* contextMenu, Model* model, const Util::Set<int>& selectedItems);
	uint16_t calcTabBarContextMenuEntries(const bool isTemporary, const bool hasChanges, const bool isLocked,
	                                      const int trackCount, const int tabCount);
} // Playlist

#endif //SAYONARA_PLAYER_CONTEXTMENUCONFIGURATOR_H
