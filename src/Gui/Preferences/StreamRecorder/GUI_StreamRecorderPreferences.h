/* GUI_StreamRecorderPreferences.h

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
 * May 13, 2012
 *
 */

#ifndef GUI_STREAMRECORDER_PREFERENCES_H
#define GUI_STREAMRECORDER_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"

#include <QPushButton>

UI_FWD(GUI_StreamRecorderPreferences)


class TagButton : public
		Gui::WidgetTemplate<QPushButton>
{
	Q_OBJECT
	PIMPL(TagButton)

public:
	TagButton(const QString& tag_name, QWidget* parent);
	~TagButton() override;

protected:
	void language_changed() override;
};


class GUI_StreamRecorderPreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_StreamRecorderPreferences)
	UI_CLASS(GUI_StreamRecorderPreferences)

	public:
		explicit GUI_StreamRecorderPreferences(const QString& identifier);
		~GUI_StreamRecorderPreferences() override;

		bool commit() override;
		void revert() override;

		QString action_name() const override;

	protected:
		void init_ui() override;
		void retranslate_ui() override;
		void skin_changed() override;

		QString error_string() const override;

	private slots:
		void sl_cb_activate_toggled(bool);
		void sl_btn_path_clicked();
		void sl_btn_default_clicked();
		void sl_line_edit_changed(const QString& new_text);
};

#endif /* GUI_StreamRecorderPreferences_H_ */

