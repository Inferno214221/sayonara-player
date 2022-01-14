/* Settings.cpp */

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

#include "Utils/Library/LibraryNamespaces.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingRegistry.h"
#include "Utils/typedefs.h"

#include "Utils/Crypt.h"
#include "Utils/Utils.h"
#include "Utils/Language/LanguageUtils.h"

#include <array>

struct Settings::Private
{
	QString version;
	std::array<AbstrSetting*, static_cast<int>(SettingKey::Num_Setting_Keys)> settings;
	bool initialized{false};

	Private() // NOLINT
	{
		settings.fill(nullptr);
	}

	~Private()
	{
		for(auto* setting : settings)
		{
			delete setting;
		}
	}
};

Settings::Settings()
{
	m = Pimpl::make<Private>();
}

Settings::~Settings() = default;

AbstrSetting* Settings::setting(SettingKey key) const
{
	return m->settings[static_cast<size_t>(key)];
}

const SettingArray& Settings::settings()
{
	return m->settings;
}

void Settings::registerSetting(AbstrSetting* s)
{
	const auto key = s->getKey();
	const auto index = static_cast<uint>(key);
	m->settings[index] = s;
}

bool Settings::checkSettings()
{
	if(m->initialized)
	{
		return true;
	}

	SettingRegistry::init();

	const auto hasUnitializedSetting = std::any_of(m->settings.begin(), m->settings.end(), [](auto* setting) {
		return (setting == nullptr);
	});

	m->initialized = (!hasUnitializedSetting);

	return m->initialized;
}

void Settings::applyFixes()
{
	const auto settingsRevision = this->get<Set::Settings_Revision>();
	if(settingsRevision < 1)
	{
		// Create Crypt keys
		const auto privateKey = ::Util::randomString(32).toLocal8Bit();
		this->set<Set::Player_PrivId>(privateKey);

		const auto publicKey = ::Util::randomString(32).toLocal8Bit();
		this->set<Set::Player_PublicId>(publicKey);

		// Crypt Last FM password
		const auto lastFmCredentials = this->get<Set::LFM_Login>();
		this->set<Set::LFM_Username>(lastFmCredentials.first);
		this->set<Set::LFM_Password>(Util::Crypt::encrypt(lastFmCredentials.second));
		this->set<Set::LFM_Login>(StringPair("", ""));

		// Crypt Proxy Password
		const auto proxyPassword = this->get<Set::Proxy_Password>();
		this->set<Set::Proxy_Password>(Util::Crypt::encrypt(proxyPassword));

		this->set<Set::Settings_Revision>(1);
	}

	if(settingsRevision < 2)
	{
		const auto language = this->get<Set::Player_Language>();
		const auto fourLetter = Util::Language::convertOldLanguage(language);
		this->set<Set::Player_Language>(fourLetter);

		this->set<Set::Settings_Revision>(2);
	}

	if(settingsRevision < 3)
	{
		const auto showAlbumCovers = this->get<Set::Lib_ShowAlbumCovers>();
		const auto coverViewType = (showAlbumCovers)
			? ::Library::ViewType::CoverView
			: ::Library::ViewType::Standard;
		this->set<Set::Lib_ViewType>(coverViewType);
		this->set<Set::Settings_Revision>(3);
	}

	if(settingsRevision < 4)
	{
		auto path = this->get<Set::Cover_TemplatePath>();
		path.replace("jpg", "png", Qt::CaseInsensitive);
		this->set<Set::Cover_TemplatePath>(path);

		this->set<Set::Settings_Revision>(4);
	}

	if(settingsRevision < 5)
	{
		auto path = this->get<Set::Cover_TemplatePath>();
		path.replace(QRegExp("\\.[a-zA-Z]{3}$"), "");
		this->set<Set::Cover_TemplatePath>(path);

		this->set<Set::Settings_Revision>(5);
	}

	if(get<Set::Player_PrivId>().isEmpty())
	{
		const auto privateId = ::Util::randomString(32).toLocal8Bit();
		this->set<Set::Player_PrivId>(privateId);
	}

	if(get<Set::Player_PublicId>().isEmpty())
	{
		const auto publicId = ::Util::randomString(32).toLocal8Bit();
		this->set<Set::Player_PublicId>(publicId);
	}
}

