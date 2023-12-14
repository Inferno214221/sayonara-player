/* SettingNotifier.h */

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

#ifndef SAYONARA_PLAYER_SETTINGNOTIFIER_H
#define SAYONARA_PLAYER_SETTINGNOTIFIER_H

#include <functional>
#include <QObject>

#pragma once

class AbstrSettingNotifier : public QObject
{
	Q_OBJECT

signals:
	void sigValueChanged();

public:
	template<typename T>
	void addListener(T* c, void (T::*fn)())
	{
		connect(this, &AbstrSettingNotifier::sigValueChanged, c, fn);
	}

	void emit_value_changed();
};

template<typename KeyClass>
class SettingNotifier
{
private:
	AbstrSettingNotifier* m=nullptr;

	SettingNotifier() :
		m(new AbstrSettingNotifier())
	{}

	SettingNotifier(const SettingNotifier& other) = delete;

public:
	~SettingNotifier() = default;

	static SettingNotifier<KeyClass>* instance()
	{
		static SettingNotifier<KeyClass> inst;
		return &inst;
	}

	void valueChanged()
	{
		m->emit_value_changed();
	}

	template<typename T>
	void addListener(T* c, void (T::*fn)())
	{
		m->addListener(c, fn);
	}
};


namespace Set
{
	template<typename KeyClassInstance, typename T>
	//typename std::enable_if<std::is_base_of<SayonaraClass, T>::value, void>::type
	void
	listen(T* t, void (T::*fn)(), bool run=true)
	{
		SettingNotifier<KeyClassInstance>::instance()->addListener(t, fn);

		if(run)
		{
			auto callable = std::bind(fn, t);
			callable();
		}
	}

	template<typename KeyClassInstance>
	void shout()
	{
		SettingNotifier<KeyClassInstance>::instance()->valueChanged();
	}
}

#endif // SAYONARA_PLAYER_SETTINGNOTIFIER_H
