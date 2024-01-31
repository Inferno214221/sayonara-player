/* GUI_EnginePreferences.h */

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

#ifndef GUI_ENGINEPREFERENCES_H
#define GUI_ENGINEPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"
#include <QProcess>

UI_FWD(GUI_EnginePreferences)

class GUI_EnginePreferences :
	public Preferences::Base
{
	Q_OBJECT
	PIMPL(GUI_EnginePreferences)
	UI_CLASS_SHARED_PTR(GUI_EnginePreferences)

	public:
		explicit GUI_EnginePreferences(const QString& identifier);
		~GUI_EnginePreferences() override;

		[[nodiscard]] QString actionName() const override;
		bool commit() override;
		void revert() override;
		void initUi() override;
		void retranslate() override;

	private slots:
		void radioButtonChanged(bool b);
		void alsaProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
		void alsaProcessErrorOccured(QProcess::ProcessError error);
		void alsaStdoutWritten();
};

#endif // GUI_ENGINEPREFERENCES_H
