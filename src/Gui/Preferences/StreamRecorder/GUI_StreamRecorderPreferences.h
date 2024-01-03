/* GUI_StreamRecorderPreferences.h

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
 * May 13, 2012
 *
 */

#ifndef GUI_STREAMRECORDER_PREFERENCES_H
#define GUI_STREAMRECORDER_PREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include "Utils/Pimpl.h"
#include "Utils/FileSystem.h"

#include <QPushButton>

UI_FWD(GUI_StreamRecorderPreferences)

namespace Util
{
	class FileSystem;
}

class TagButton :
	public Gui::WidgetTemplate<QPushButton>
{
	Q_OBJECT
	PIMPL(TagButton)

	public:
		TagButton(const QString& tagName, QWidget* parent);
		~TagButton() override;

	protected:
		void languageChanged() override;
};

class GUI_StreamRecorderPreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_StreamRecorderPreferences)
	UI_CLASS(GUI_StreamRecorderPreferences)

	public:
		GUI_StreamRecorderPreferences(const QString& identifier, const std::shared_ptr<Util::FileSystem>& filesystem);
		~GUI_StreamRecorderPreferences() override;

		bool commit() override;
		void revert() override;

		[[nodiscard]] QString actionName() const override;

	protected:
		void initUi() override;
		void retranslate() override;
		void skinChanged() override;

		[[nodiscard]] QString errorString() const override;

	private slots:
		void activeToggled(bool);
		void pathButtonClicked();
		void defaultButtonClicked();
		void lineEditChanged(const QString& text);
};

#endif /* GUI_StreamRecorderPreferences_H_ */

