/* GUI_TagEdit.h */

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

#ifndef GUI_TAGEDIT_H_
#define GUI_TAGEDIT_H_

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

/**
 * @brief The GUI_TagEdit class
 * @ingroup GuiTagging
 */
namespace Tagging
{
	class Editor;
}

class QAbstractButton;
class MetaDataList;
class MetaData;

UI_FWD(GUI_TagEdit)

class GUI_TagEdit :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_TagEdit)
	PIMPL(GUI_TagEdit)

	signals:
		void sigOkClicked(const MetaDataList&);
		void sigCancelled();

	public:
		explicit GUI_TagEdit(QWidget* parent=nullptr);
		~GUI_TagEdit() override;

		void showDefaultTab();
		void showCoverTab();

		void setMetadata(const MetaDataList& tracks);

	private slots:
		void nextButtonClicked();
		void prevButtonClicked();
		void undoClicked();
		void undoAllClicked();
		void loadEntireAlbumClicked();

		void metadataChanged(const MetaDataList& tracks);
		void applyTagFromPathTriggered();
		void applyAllTagFromPathTriggered();

		void commit();
		void commitStarted();
		void commitFinished();
		void progressChanged(int val);

	private:
		void setCurrentIndex(int index);
		void refreshCurrentTrack();
		void reset();
		void writeChanges(int trackIndex);
		void runEditor(Tagging::Editor* editor);

	protected:
		void showEvent(QShowEvent* e) override;
		void languageChanged() override;
};

#endif
