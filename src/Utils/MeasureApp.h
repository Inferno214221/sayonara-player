/* MeasureApp.h */
/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_MEASUREAPP_H
#define SAYONARA_PLAYER_MEASUREAPP_H

#include "Utils/Pimpl.h"

#include <functional>

class QElapsedTimer;
class QString;
namespace Util
{
	class MeasureApp
	{
		PIMPL(MeasureApp)

		public:
			MeasureApp(const QString& component, QElapsedTimer* timer);
			~MeasureApp() noexcept;

			MeasureApp(const MeasureApp& other) = delete;
			MeasureApp(MeasureApp&& other) = delete;
			MeasureApp& operator=(const MeasureApp& other) = delete;
			MeasureApp& operator=(MeasureApp&& other) = delete;
	};

	QElapsedTimer* startMeasure();
	void measure(const QString& component, QElapsedTimer* timer, std::function<void(void)>&& task);
}
#endif //SAYONARA_PLAYER_MEASUREAPP_H
