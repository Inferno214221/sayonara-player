/* GUI_ConfigureStation.h */

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

#ifndef GUI_ConfigureStation_H
#define GUI_ConfigureStation_H

#include "Gui/Utils/Widgets/Dialog.h"

#include "Utils/Pimpl.h"
#include "Utils/Streams/Station.h"

UI_FWD(GUI_ConfigureStation)

class GUI_ConfigureStation :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_ConfigureStation)
	UI_CLASS_SHARED_PTR(GUI_ConfigureStation)

	public:
		enum Mode
		{
			New,
			Edit,
			Save
		};

		explicit GUI_ConfigureStation(QWidget* parent = nullptr);
		~GUI_ConfigureStation() override;

		void initUi();

		void setError(const QString& message);
		void setMode(const QString& streamName, Mode mode);
		[[nodiscard]] Mode mode() const;

		virtual StationPtr configuredStation() = 0;
		virtual QList<QWidget*> configurationWidgets() = 0;
		virtual void configureWidgets(StationPtr station) = 0;
		[[nodiscard]] virtual QString labelText(int row) const = 0;

	protected:
		void languageChanged() override;
};

#endif // GUI_ConfigureStation_H
