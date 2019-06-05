/* PlaylistMode.h */

/* Copyright (C) 2011  Lucio Carreras
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

#ifndef PLAYLISTMODE_H_
#define PLAYLISTMODE_H_

#include "Utils/Pimpl.h"
#include "Utils/Settings/SettingConvertible.h"

namespace Playlist
{
	/**
	 * @brief The Mode class
	 * @ingroup PlaylistHelper
	 */
	class Mode :
		public SettingConvertible
	{
		PIMPL(Mode)

		public:
			enum State
			{
				Off  = 0,
				On   = 1,
				Disabled = 2 // this has to be because of consistence
			};

		public:
			Mode();
			~Mode();
			Mode(const Mode& other);
			Mode& operator=(const Mode& other);

			Playlist::Mode::State rep1() const;
			Playlist::Mode::State repAll() const;
			Playlist::Mode::State append() const;
			Playlist::Mode::State shuffle() const;
			Playlist::Mode::State dynamic() const;
			Playlist::Mode::State gapless() const;

			void setRep1(Playlist::Mode::State state);
			void setRepAll(Playlist::Mode::State state);
			void setAppend(Playlist::Mode::State state);
			void setShuffle(Playlist::Mode::State state);
			void setDynamic(Playlist::Mode::State state);
			void setGapless(Playlist::Mode::State state);

			void setRep1(bool on, bool enabled=true);
			void setRepAll(bool on, bool enabled=true);
			void setAppend(bool on, bool enabled=true);
			void setShuffle(bool on, bool enabled=true);
			void setDynamic(bool on, bool enabled=true);
			void setGapless(bool on, bool enabled=true);

			static bool isActive(Playlist::Mode::State pl);
			static bool isEnabled(Playlist::Mode::State pl);
			static bool isActiveAndEnabled(Playlist::Mode::State pl);

			void print();

			QString toString() const override;
			bool loadFromString(const QString& str) override;

			bool operator==(const Playlist::Mode& pm) const;
	};
}

#endif
