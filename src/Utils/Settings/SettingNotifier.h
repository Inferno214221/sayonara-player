/* SettingNotifier.h */

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

#ifndef SETTINGNOTIFIER_H
#define SETTINGNOTIFIER_H

#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Settings/SettingKeyEnum.h"

#include <functional>
#include <iostream>
#include <QObject>

#pragma once


class SettingNotifier :
	public QObject
{
	Q_OBJECT

	signals:
		void sig_value_changed(SettingKey);

	private:
		SettingNotifier();
		SettingNotifier(const SettingNotifier& other) = delete;

	public:
		~SettingNotifier();

		static SettingNotifier* instance();
		void change_value(SettingKey key);

		template<typename T>
		void add_listener(SettingKey key, T* t, void (T::*fn)())
		{
			connect(this, &SettingNotifier::sig_value_changed, this, [=](SettingKey k)
			{
				if(key == k)
				{
					auto callable = std::bind(fn, t);
					callable();
				}
			});
		}
};



#endif // SETTINGNOTIFIER_H
