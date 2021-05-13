/* GUIClass.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef GUICLASS_H
#define GUICLASS_H

#include <memory>

#define UI_FWD(x) namespace Ui { class x ; }
#define UI_CLASS(x) private: \
	Ui:: x *ui=nullptr;
#define UI_CLASS_SHARED_PTR(x) private: \
	std::shared_ptr<Ui:: x> ui{nullptr};

#endif // GUICLASS_H
