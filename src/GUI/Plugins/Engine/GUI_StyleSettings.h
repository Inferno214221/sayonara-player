/* GUI_StyleSettings.h */

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

#ifndef STYLESETTINGS_H
#define STYLESETTINGS_H

#include "Utils/Pimpl.h"
#include "GUI/Utils/GuiClass.h"
#include "GUI/Utils/Widgets/Dialog.h"

UI_FWD(GUI_Style)

class QSpinBox;
class GUI_StyleSettings :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_StyleSettings)
	UI_CLASS(GUI_Style)

public:
	explicit GUI_StyleSettings(QWidget *parent=nullptr);
	virtual ~GUI_StyleSettings();

signals:
	void sig_style_update();

public slots:
	void show(int);


private slots:
	void combo_styles_changed(int);
	void combo_text_changed(const QString&);
	void col1_activated();
	void col2_activated();
	void col3_activated();
	void col4_activated();
	void save_pressed();
	void del_pressed();
	void undo_pressed();
	void spin_box_changed(int);
	void col_changed();


private:
	void init();

	void connect_spinbox(const QSpinBox* box);
	void disconnect_spinbox(const QSpinBox* box);
	void connect_spinboxes();
	void disconnect_spinboxes();
	void set_sth_changed(bool b);

protected:
	void closeEvent(QCloseEvent* event) override;
	void language_changed() override;

	void connect_combo_idx_changed();
	void disconnect_combo_idx_changed();
};

#endif // STYLESETTINGS_H
