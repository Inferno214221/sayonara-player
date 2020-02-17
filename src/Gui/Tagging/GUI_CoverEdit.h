/* GUI_CoverEdit.h */

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



#ifndef GUI_COVEREDIT_H
#define GUI_COVEREDIT_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_CoverEdit)

namespace Tagging
{
	class Editor;
}

class MetaData;
class MetaDataList;
class QPixmap;
class GUI_TagEdit;

/**
 * @brief The GUI_CoverEdit class
 * @ingroup GuiTagging
 */
class GUI_CoverEdit :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_CoverEdit)
	UI_CLASS(GUI_CoverEdit)

	public:
		/**
		 * @brief GUI_CoverEdit
		 * @param editor The same tag editor as used in GUI_TagEdit
		 * @param parent
		 */
		explicit GUI_CoverEdit(GUI_TagEdit* parent);
		~GUI_CoverEdit() override;

		/**
		 * @brief Shows the current cover (if there) and offers to replace it
		 * Every other widget is hidden
		 */
		void reset();

		/**
		 * @brief refetches the track from the tag editor
		 * and sets the cover to the left button
		 */
		void refreshCurrentTrack();

		/**
		 * @brief sets the current index for a track which is currently processed
		 * @param index
		 */
		void setCurrentIndex(int index);

		/**
		 * @brief returns the new cover for a current track.
		 * @param index
		 * @return empty pixmap if index is invalid, or no new cover is desired a track
		 */
		QPixmap selectedCover(int index) const;

	private:
		void setCover(const MetaData& md);
		void showReplacementField(bool b);
		bool isCoverReplacementActive() const;

	private slots:
		/**
		 * @brief When button has finished setting up its button
		 */
		void coverChanged();
		void replaceToggled(bool b);
		void coverAllToggled(bool b);
		void setMetadata(const MetaDataList& v_md);

	protected:
		void languageChanged() override;
};

#endif // GUI_COVEREDIT_H
