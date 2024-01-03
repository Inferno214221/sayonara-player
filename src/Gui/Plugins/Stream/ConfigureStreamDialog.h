/* ConfigureStreamDialog.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef GUI_CONFIGURESTREAM_H
#define GUI_CONFIGURESTREAM_H

#include "GUI_ConfigureStation.h"

class ConfigureStreamDialog :
	public GUI_ConfigureStation
{
	PIMPL(ConfigureStreamDialog)

	public:
		explicit ConfigureStreamDialog(QWidget* parent);
		~ConfigureStreamDialog() override;

		[[nodiscard]] StationPtr configuredStation() override;

		void configureWidgets(StationPtr station) override;
		[[nodiscard]] QList<QWidget*> configurationWidgets() override;

		[[nodiscard]] QString labelText(int index) const override;
};

#endif // GUI_CONFIGURESTREAM_H
