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
#pragma once

#include <QObject>

#include <functional>
#include <memory>

class AbstrSettingNotifier :
	public QObject
{
	Q_OBJECT

	signals:
		void sigValueChanged();

	public:
		template<typename Listener>
		void addListener(Listener* listener, void (Listener::*fn)())
		{
			connect(this, &AbstrSettingNotifier::sigValueChanged, listener, fn);
		}

		void emitValueChanged();
};

template<typename KeyClass>
class SettingNotifier
{
	public:
		SettingNotifier(const SettingNotifier& other) = delete;
		SettingNotifier(SettingNotifier&& other) = delete;
		SettingNotifier& operator=(const SettingNotifier& other) = delete;
		SettingNotifier& operator=(SettingNotifier&& other) = delete;

		~SettingNotifier() = default;

		static SettingNotifier<KeyClass>* instance()
		{
			static SettingNotifier<KeyClass> inst;
			return &inst;
		}

		void valueChanged()
		{
			m_settingNotifier->emitValueChanged();
		}

		template<typename T>
		void addListener(T* c, void (T::*fn)())
		{
			m_settingNotifier->addListener(c, fn);
		}

	private:
		SettingNotifier() = default;

		std::unique_ptr<AbstrSettingNotifier> m_settingNotifier {std::make_unique<AbstrSettingNotifier>()};
};

namespace Set
{
	template<typename KeyClass, typename Listener>
	void listen(Listener* t, void (Listener::*fn)(), const bool run = true)
	{
		SettingNotifier<KeyClass>::instance()->addListener(t, fn);

		if(run)
		{
			auto callable = std::bind(fn, t);
			callable();
		}
	}

	template<typename KeyClass>
	void shout()
	{
		SettingNotifier<KeyClass>::instance()->valueChanged();
	}
}

#endif // SAYONARA_PLAYER_SETTINGNOTIFIER_H
