/* GUI_TagFromPath.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
		GUI_TagFromPath(Tagging::Editor* editor, QWidget* parent);
		~GUI_TagFromPath();

		void addInvalidFilepath(const QString& filepath);
		void clearInvalidFilepaths();

		QString getRegexString() const;

		void setCurrentIndex(int index);
		void refreshCurrentTrack();
		void reset();

	private:
		bool checkIfAnyButtonIsChecked() const;
		void initButtons();
		void setTagColors(bool valid);
		bool replaceSelectedTagText(Tagging::TagName tagName, bool buttonChecked);
		void btnChecked(QPushButton* btn, bool b, Tagging::TagName tagName);
		void showErrorFrame(bool b);

	private slots:
		void btnTagHelpClicked();
		void tagTextChanged(const QString& tagString);

	protected:
		void showEvent(QShowEvent* event) override;
		void languageChanged() override;
};

#endif // GUI_TAGFROMPATH_H
