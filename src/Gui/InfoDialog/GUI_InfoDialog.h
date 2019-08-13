/* GUI_InfoDialog.h

 * Copyright (C) 2011-2019 Lucio Carreras
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
 * created by Lucio Carreras,
 * Jul 19, 2012
 *
 */

#ifndef GUI_INFODIALOG_H_
#define GUI_INFODIALOG_H_

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class GUI_TagEdit;
class GUI_Lyrics;
class InfoDialogContainer;
namespace Cover
{
	class Location;
}


UI_FWD(InfoDialog)

/**
 * @brief The GUI_InfoDialog class
 * @ingroup InfoDialog
 */
class GUI_InfoDialog :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_InfoDialog)
	UI_CLASS(InfoDialog)

public:

	enum class Tab : uint8_t
	{
		Info=0,
		Lyrics=1,
		Edit=2
	};

	explicit GUI_InfoDialog(InfoDialogContainer* container, QWidget* parent=nullptr);
	~GUI_InfoDialog();

	void set_metadata(const MetaDataList& vd, MD::Interpretation interpretation);
	bool has_metadata() const;

	GUI_InfoDialog::Tab show(GUI_InfoDialog::Tab tab);
	void show_cover_edit_tab();

protected:
	void skin_changed() override;
	void language_changed() override;

private slots:
	void tab_index_changed_int(int idx);
	void tab_index_changed(GUI_InfoDialog::Tab idx);
	void write_cover_to_tracks_clicked();
	void cover_changed();

private:
	void init();

	void init_tag_edit();
	void init_lyrics();

	void show_info_tab();
	void show_lyrics_tab();
	void show_tag_edit_tab();

	void prepare_cover(const Cover::Location& cover_path);
	void prepare_info(MD::Interpretation mode);

	void closeEvent(QCloseEvent *e) override;
	void showEvent(QShowEvent *e) override;

	using Gui::Dialog::show;
};

#endif /* GUI_INFODIALOG_H_ */
