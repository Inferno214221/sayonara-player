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

#include <functional>
#include <QObject>

#pragma once

class AbstrSettingNotifier : public QObject
{
	Q_OBJECT

signals:
	void sig_value_changed();

public:
	template<typename T>
	void add_listener(T* c, void (T::*fn)())
	{
		connect(this, &AbstrSettingNotifier::sig_value_changed, c, fn);
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
	~SettingNotifier() {}

	static SettingNotifier<KeyClass>* instance()
	{
		static SettingNotifier<KeyClass> inst;
		return &inst;
	}

	void val_changed()
	{
		m->emit_value_changed();
	}

	template<typename T>
	void add_listener(T* c, void (T::*fn)())
	{
		m->add_listener(c, fn);
	}
};


namespace Set
{
	template<typename KeyClassInstance, typename T>
	//typename std::enable_if<std::is_base_of<SayonaraClass, T>::value, void>::type
	void
	listen(T* t, void (T::*fn)(), bool run=true)
	{
		SettingNotifier<KeyClassInstance>::instance()->add_listener(t, fn);

		if(run)
		{
			auto callable = std::bind(fn, t);
			callable();
		}
	}

	template<typename KeyClassInstance>
	void shout()
	{
		SettingNotifier<KeyClassInstance>::instance()->val_changed();
	}
}


#endif // SETTINGNOTIFIER_H
