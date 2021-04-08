/* CoverButton.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef COVER_BUTTON_H
#define COVER_BUTTON_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Widgets/ImageButton.h"

namespace Cover
{
	class Location;
}

namespace Gui
{
	/**
	 * @brief The CoverButton class
	 * @ingroup GuiCovers
	 */
	class CoverButton :
		public Gui::WidgetTemplate<Gui::ImageButton>
	{
		Q_OBJECT
		PIMPL(CoverButton)

		signals:
			void sigCoverChanged();
			void sigRejected();

		public:
			explicit CoverButton(QWidget* parent = nullptr);
			~CoverButton() override;

			/**
			 * @brief Set an appropriate cover location.
			 * Afterwards a search is triggered to find the cover.
			 * @param coverLocation
			 */
			void setCoverLocation(const Cover::Location& coverLocation);

			/**
			 * @brief silent results that the cover is not stored
			 * productively. The AlternativeCoverFetcher will
			 * save the cover to a temporary path which can be re-
			 * trieved by Cover::Location::alternative_path()
			 * @param silent
			 */
			void setSilent(bool silent);
			bool isSilent() const;

			void setAlternativeSearchEnabled(bool b);
			bool isAlternativeSearchEnabled() const;

		public slots:
			void trigger();

		private slots:
			void alternativeCoverFetched(const Cover::Location& coverLocation);
			void coverLookupFinished(bool success);
			void coversChanged();

		private:
			void coverFadingChanged();

		protected:
			void languageChanged() override;
	};
}

#endif
