/* ContextMenuConfigurator.cpp, (Created on 12.01.2024) */

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

#include "ContextMenuConfigurator.h"
#include "PlaylistContextMenu.h"
#include "PlaylistMenuEntry.h"
#include "PlaylistModel.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Set.h"
#include "Utils/MetaData/MetaDataList.h"

namespace Playlist
{
	ContextMenu::Entries
	calcContextMenuEntries(ContextMenu* contextMenu, Model* model, const Util::Set<int>& selectedItems)
	{
		auto entryMask = ContextMenu::Entries {ContextMenu::EntryNone};
		const auto isLocked = model->isLocked();
		const auto isRearrangeAllowed = !isLocked || GetSetting(Set::PL_ModificatorAllowRearrangeMethods);

		if(model->rowCount() > 0)
		{
			entryMask |= (ContextMenu::EntryRefresh |
			              (isLocked ? 0 : ContextMenu::EntryClear) |
			              (!isRearrangeAllowed ? 0 : ContextMenu::EntryReverse) |
			              (!isRearrangeAllowed ? 0 : ContextMenu::EntryRandomize) |
			              (!isRearrangeAllowed ? 0 : ContextMenu::EntrySort) |
			              ContextMenu::EntryJumpToNextAlbum);

			if(!selectedItems.isEmpty())
			{
				entryMask |= (ContextMenu::EntryPlay |
				              ContextMenu::EntryInfo |
				              (isLocked ? 0 : ContextMenu::EntryRemove));

				const auto firstSelectedRow = *selectedItems.begin();
				if((selectedItems.size() == 1) && (firstSelectedRow < model->rowCount()))
				{
					const auto tracks = model->metadata(selectedItems);
					const auto& track = tracks[0];

					entryMask |= contextMenu->setTrack(track, (firstSelectedRow == model->currentTrack()));
				}

				if(model->hasLocalMedia(selectedItems))
				{
					entryMask |= (ContextMenu::EntryEdit |
					              ContextMenu::EntryDelete);
				}
			}

			if(model->currentTrack() >= 0)
			{
				entryMask |= ContextMenu::EntryCurrentTrack;
			}
		}

		return entryMask;
	}

	uint16_t calcTabBarContextMenuEntries(const bool isTemporary, const bool hasChanges, const bool isLocked,
	                                      const int trackCount, const int tabCount)
	{
		const auto saveEnabled = (!isTemporary);
		const auto saveAsEnabled = true;
		const auto saveToFileEnabled = (trackCount > 0);
		const auto deleteEnabled = (!isTemporary);
		const auto resetEnabled = (!isTemporary && hasChanges);
		const auto closeEnabled = (tabCount > 2);
		const auto clearEnabled = (trackCount > 0) && !isLocked;

		auto entries = MenuEntries {MenuEntry::None};

		entries |= (saveEnabled) ? MenuEntry::Save : 0;
		entries |= (saveAsEnabled) ? MenuEntry::SaveAs : 0;
		entries |= (saveToFileEnabled) ? MenuEntry::SaveToFile : 0;
		entries |= (deleteEnabled) ? MenuEntry::Delete : 0;
		entries |= (resetEnabled) ? MenuEntry::Reset : 0;
		entries |= (closeEnabled) ? MenuEntry::Close : 0;
		entries |= (closeEnabled) ? MenuEntry::CloseOthers : 0;
		entries |= (clearEnabled) ? MenuEntry::Clear : 0;
		entries |= MenuEntry::OpenFile;
		entries |= MenuEntry::OpenDir;
		entries |= MenuEntry::Rename;
		entries |= (isLocked) ? MenuEntry::Unlock : MenuEntry::Lock;

		return entries;
	}
} // Playlist
