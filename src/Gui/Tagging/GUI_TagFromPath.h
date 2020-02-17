/* GUI_TagFromPath.h */

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

#ifndef GUI_TAGFROMPATH_H
#define GUI_TAGFROMPATH_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/Utils/GuiClass.h"
#include "Utils/Pimpl.h"

#include "Components/Tagging/Expression.h"

UI_FWD(GUI_TagFromPath)

class QPushButton;

namespace Tagging
{
	class Editor;
}
class MetaData;

/**
 * @brief The GUI_TagFromPath class
 * @ingroup GuiTagging
 */
class GUI_TagFromPath :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_TagFromPath)
	PIMPL(GUI_TagFromPath)

	signals:
		void sigApply();
		void sigApplyAll();

	public:
		GUI_TagFromPath(QWidget* parent=nullptr);
		~GUI_TagFromPath();

		void setFilepath(const QString& filepath);

		/**
		 * @brief add a filepath where the regex could not be applied on
		 * @param filepath
		 */
		void addInvalidFilepath(const QString& filepath);
		void clearInvalidFilepaths();

		QString getRegexString() const;
		void reset();

	private:
		/**
		 * @brief sets red if not valid
		 * @param valid if tag is valid or not
		 */
		void setTagColors(bool valid);
		bool replaceSelectedTagText(Tagging::TagName t, bool b);
		void btnChecked(QPushButton* btn, bool b, Tagging::TagName tagName);
		void showErrorFrame(bool b);

	private slots:
		/**
		 * @brief calls webpage with help
		 */
		void btnTagHelpClicked();

		/**
		 * @brief tries to apply the tag
		 */
		void tagTextChanged(const QString& newText);

		void btnTitleChecked(bool b);
		void btnArtistChecked(bool b);
		void btnAlbumChecked(bool b);
		void btnTrackNrChecked(bool b);
		void btnDiscnumberChecked(bool b);
		void btnYearChecked(bool b);

	protected:
		void languageChanged();
};

#endif // GUI_TAGFROMPATH_H
