/* LyricsLibraryContainer.h, (Created on 04.01.2024) */

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
#ifndef SAYONARA_PLAYER_LYRICSLIBRARYCONTAINER_H
#define SAYONARA_PLAYER_LYRICSLIBRARYCONTAINER_H

#include "Components/LibraryManagement/LibraryContainer.h"
#include "Gui/Library/LibraryContainer.h"

#include "Utils/Pimpl.h"

class PlayManager;
class MetaData;

namespace Library
{
	class PluginHandler;
}

namespace Gui::Library
{
	class Container;
}

class LyricsLibraryContainer :
	public Gui::Library::Container
{
	PIMPL(LyricsLibraryContainer)

	public:
		LyricsLibraryContainer(PlayManager* playManager, Library::PluginHandler* pluginHandler);
		~LyricsLibraryContainer() override = default;

		[[nodiscard]] QFrame* header() const override;
		[[nodiscard]] QIcon icon() const override;
		[[nodiscard]] QMenu* menu() override;
		[[nodiscard]] QString displayName() const override;
		[[nodiscard]] QString name() const override;
		[[nodiscard]] QWidget* widget() const override;
		[[nodiscard]] bool isLocal() const override;
		void rename(const QString& newName) override;
	protected:
		void initUi() override;
};

#endif //SAYONARA_PLAYER_LYRICSLIBRARYCONTAINER_H
