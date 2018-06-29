/* GUI_Controls.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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



#ifndef GUI_CONTROLS_H
#define GUI_CONTROLS_H

#include <QWidget>

#include "Components/PlayManager/PlayState.h"

#include "Utils/Pimpl.h"
#include "Utils/MetaData/RadioMode.h"

#include "GUI/Utils/Shortcuts/ShortcutWidget.h"
#include "GUI/Utils/Widgets/Widget.h"
#include "GUI/InfoDialog/InfoDialogContainer.h"
#include "GUI/Utils/Icons.h"

UI_FWD(GUI_Controls)

class MetaData;
class MetaDataList;

class GUI_Controls :
		public Gui::Widget,
		public ShortcutWidget,
		public InfoDialogContainer
{
	Q_OBJECT
	PIMPL(GUI_Controls)
	UI_CLASS(GUI_Controls)

private:
	void played();
	void paused();
	void stopped();

	void check_record_button_visible();

	void set_cover_location(const MetaData &md);
	void set_standard_cover();
	void set_radio_mode(RadioMode radio);

	void setup_volume_button(int percent);

	QString get_cur_pos_string(int val) const;
	void set_cur_pos_label(int val);
	void set_total_time_label(MilliSeconds total_time);
	void set_info_labels(const MetaData &md);
	void refresh_info_labels();

	void setup_shortcuts();
	void setup_connections();


protected:
	void resizeEvent(QResizeEvent* e) override;
	void showEvent(QShowEvent* e) override;
	void contextMenuEvent(QContextMenuEvent* e) override;

public:
	explicit GUI_Controls(QWidget *parent = 0);
	~GUI_Controls();

	QString get_shortcut_text(const QString &shortcut_identifier) const override;
	void language_changed() override;
	void skin_changed() override;

	void file_info_changed();
	void sr_active_changed();

public slots:
	void change_volume_by_tick(int val);

private slots:
	void playstate_changed(PlayState state);
	void rec_changed(bool b);

	void buffering(int progress);

	void cur_pos_changed(MilliSeconds pos_ms);
	void progress_moved(int val);
	void progress_hovered(int val);

	void volume_changed(int val);

	void mute_button_clicked();
	void mute_changed(bool muted);

	void track_changed(const MetaData& md);
	void id3_tags_changed(const MetaDataList &v_md_old, const MetaDataList &v_md_new);
	void md_changed(const MetaData &md);
	void dur_changed(const MetaData &md);
	void br_changed(const MetaData &md);

	// cover changed by engine
	void force_cover(const QImage &img);


	// InfoDialogContainer interface
protected:
	MD::Interpretation metadata_interpretation() const override;
	MetaDataList info_dialog_data() const override;
};

#endif // GUI_CONTROLS_H
