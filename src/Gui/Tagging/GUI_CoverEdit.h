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

namespace Tagging
{
	class Editor;
}

class GUI_CoverEdit :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(GUI_CoverEdit)
	UI_CLASS(GUI_CoverEdit)

	public:
		GUI_CoverEdit(Tagging::Editor* tagEditor, QWidget* parent);
		~GUI_CoverEdit() override;

		void setCurrentIndex(int index);

		void refreshCurrentTrack();
		void reset();
		void updateTrack(int index);

	private:
		void refreshOriginalCover();
		void refreshReplacementCover();

	private slots:
		void coverChanged();
		void replaceToggled(bool b);
		void btnAllToggled(bool b);

	protected:
		void showEvent(QShowEvent* event) override;
		void languageChanged() override;
};

#endif // GUI_COVEREDIT_H
