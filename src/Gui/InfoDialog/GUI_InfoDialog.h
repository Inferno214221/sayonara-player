/* GUI_InfoDialog.h

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Jul 19, 2012
 *
 */

#ifndef GUI_INFODIALOG_H_
#define GUI_INFODIALOG_H_

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

namespace Cover
{
	class Location;
}

UI_FWD(InfoDialog)

class GUI_InfoDialog :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_InfoDialog)
	UI_CLASS_SHARED_PTR(InfoDialog)

	public:
		enum class Tab :
			uint8_t
		{
			Info = 0,
			Lyrics = 1,
			Edit = 2
		};

		explicit GUI_InfoDialog(QWidget* parent = nullptr);
		~GUI_InfoDialog() override;

		void setMetadata(const MetaDataList& tracks, MD::Interpretation interpretation);
		[[nodiscard]] bool hasMetadata() const;
		void setBusy(bool b);

		GUI_InfoDialog::Tab show(GUI_InfoDialog::Tab track);
		void showCoverEditTab();

	protected:
		void skinChanged() override;
		void languageChanged() override;
		void closeEvent(QCloseEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;

	private slots:
		void tabIndexChanged(int idx);
		void writeCoversToTracksClicked();
		void coverChanged();

	private: // NOLINT(readability-redundant-access-specifiers)
		void init();
		void initTagEdit();
		void initLyrics();

		void showInfoTab();
		void showLyricsTab();
		void showTagEditTab();

		void prepareCover(const Cover::Location& coverLocation);
		void prepareInfo(MD::Interpretation mode);
		void enableTabs();
		void switchTab(Tab tab);
		using Gui::Dialog::show;
};

#endif /* GUI_INFODIALOG_H_ */
