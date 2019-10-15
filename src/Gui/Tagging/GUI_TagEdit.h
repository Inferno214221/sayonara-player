/* GUI_TagEdit.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Components/Tagging/Expression.h"
#include "Utils/Pimpl.h"

/**
 * @brief The GUI_TagEdit class
 * @ingroup GuiTagging
 */
namespace Tagging
{
	class Editor;
}

class MetaDataList;
class MetaData;

UI_FWD(GUI_TagEdit)

class GUI_TagEdit :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_TagEdit)
	PIMPL(GUI_TagEdit)

public:
	explicit GUI_TagEdit(QWidget* parent=nullptr);
	~GUI_TagEdit();

	/**
	 * @brief Commits changes to db/file
	 */
	void commit();

	/**
	 * @brief calls undo_all, and closes the entire dialog
	 */
	void cancel();

	/**
	 * @brief shows/hides the close button. We don't need a close button
	 * when this widget is part of another
	 * @param show
	 */
	void show_close_button(bool show);

	/**
	 * @brief Directly go to the cover tab
	 */
	void show_cover_tab();

	void set_metadata(const MetaDataList& v_md);
	int count() const;

	Tagging::Editor* editor() const;


signals:
	void sig_ok_clicked(const MetaDataList&);
	void sig_undo_clicked(int idx);
	void sig_undo_all_clicked();
	void sig_cancelled();


private:
	void set_current_index(int index);
	void init_completer();


	/**
	 * @brief fills track information for current index (_cur_idx)
	 */
	void refresh_current_track();


	/**
	 * @brief resets the ui, sets the _cur_idx to -1
	 */
	void reset();


	/**
	 * @brief writes changes to the tag edit logic, does not write to db or file, also see ok_button_clicked()
	 * @param idx track index
	 */
	void write_changes(int idx);


	/**
	 * @brief checks, if current index is valid
	 * @param idx index of interest
	 * @return true if index is inside bounds, false else
	 */
	bool check_idx(int idx) const;


private slots:
	/**
	 * @brief calls write_changes and track_idx_changed with new _cur_idx
	 */
	void next_button_clicked();


	/**
	 * @brief calls write_changes and track_idx_changed with new _cur_idx
	 */
	void prev_button_clicked();


	/**
	 * @brief Undo on current track
	 */
	void undo_clicked();


	/**
	 * @brief Undo on all tracks
	 */
	void undo_all_clicked();


	/**
	 * @brief Shows progress bar if val > 0
	 * @param val value of progress bar
	 */
	void progress_changed(int val);


	/**
	 * @brief update gui, if metadata was changed
	 */
	void metadata_changed(const MetaDataList&);

	void apply_tag_from_path();

	void apply_all_tag_from_path();


	/**
	 * @brief triggered, when player language has been changed
	 */
	void language_changed() override;

	/**
	 * @brief private slots for notifying when to disable everything
	 */
	void commit_started();

	/**
	 * @brief private slot for notifying the MetaDataChangeNotifier
	 */
	void commit_finished();


	/**
	 * @brief loads the complete album for the current track
	 */
	void load_entire_album();


protected:
	void showEvent(QShowEvent* e) override;
};

#endif
