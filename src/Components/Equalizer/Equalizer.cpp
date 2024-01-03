/* Equalizer.cpp */
/*
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
#include "Equalizer.h"
#include "Interfaces/Engine/SoundModifier.h"

#include "Database/Connector.h"
#include "Database/Equalizer.h"

#include "Utils/Algorithm.h"
#include "Utils/EqualizerSetting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

namespace Algorithm = Util::Algorithm;

struct Equalizer::Private
{
	private:
		SoundModifier* mSoundModifier;

	public:
		static constexpr double scaleFactors[] = {1.0, 0.6, 0.20, 0.06, 0.01};

		DB::Equalizer* db = nullptr;

		QList<EqualizerSetting> presets;
		EqualizerSetting referenceValue;
		int currentIndex;
		bool gaussEnabled;

		Private(SoundModifier* soundModifier) :
			mSoundModifier {soundModifier},
			db {DB::Connector::instance()->equalizerConnector()},
			currentIndex(GetSetting(Set::Eq_Last)),
			gaussEnabled(GetSetting(Set::Eq_Gauss))
		{
			db->fetchAllEqualizers(presets);
			if(!Util::between(this->currentIndex, this->presets))
			{
				this->currentIndex = 0;
			}

			if(Util::between(this->currentIndex, this->presets))
			{
				applyEqualizer(presets[currentIndex]);
			}
		}

		QList<int>
		changeValue(int equalizerIndex, int band, int value, bool affectNeighbours);

		void applyEqualizer(const EqualizerSetting& preset)
		{
			if(mSoundModifier)
			{
				int band = 0;
				for(const auto& value: preset)
				{
					mSoundModifier->setEqualizer(band++, value);
				}
			}
		}
};

Equalizer::Equalizer(SoundModifier* soundModifier, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(soundModifier);
}

Equalizer::~Equalizer() noexcept = default;

QStringList Equalizer::names() const
{
	QStringList names;
	Algorithm::transform(m->presets, names, [](const auto& equalizer) {
		return (equalizer.name());
	});

	return names;
}

QStringList Equalizer::defaultNames() const
{
	const auto defaults = DB::Equalizer::factoryDefaults();

	QStringList defaultNames;
	Algorithm::transform(defaults, defaultNames, [](const auto& equalizer) {
		return equalizer.name();
	});

	return defaultNames;
}

void Equalizer::changeValue(int index, int band, int value)
{
	const auto& equalizer = equalizerSetting(index);

	const auto affectedBands =
		m->changeValue(index, band, value, m->gaussEnabled);

	if(!affectedBands.isEmpty())
	{
		if(m->db->updateEqualizer(equalizer))
		{
			for(const auto& band: affectedBands)
			{
				emit sigValueChanged(band, equalizer.value(band));
			}
		}
	}

	m->applyEqualizer(equalizer);
}

void Equalizer::resetPreset(int equalizerIndex)
{
	if(!Util::between(equalizerIndex, m->presets))
	{
		return;
	}

	const auto& equalizer = equalizerSetting(equalizerIndex);
	const auto values = equalizer.defaultValues();

	int band = 0;
	for(const auto& value: values)
	{
		m->changeValue(equalizerIndex, band, value, false);
		emit sigValueChanged(band++, value);
	}

	m->db->updateEqualizer(equalizer);
	m->applyEqualizer(equalizer);
}

Equalizer::RenameError
Equalizer::renamePreset(int index, const QString& newName)
{
	if(!Util::between(index, m->presets))
	{
		spLog(Log::Error, this) << "Rename: Index out of bounds: " << index;
		return RenameError::InvalidIndex;
	}

	if(newName.trimmed().isEmpty())
	{
		spLog(Log::Error, this) << "Rename:  Empty name not allowed";
		return RenameError::EmptyName;
	}

	const auto containsName =
		Algorithm::contains(m->presets, [newName](const auto& preset) {
			return (preset.name().trimmed() == newName.trimmed());
		});

	if(containsName)
	{
		spLog(Log::Error, this) << "Rename: Name already there";
		return RenameError::NameAlreadyKnown;
	}

	auto equalizer = equalizerSetting(index);
	equalizer.setName(newName);

	bool success = m->db->updateEqualizer(equalizer);
	if(!success)
	{
		return RenameError::DbError;
	}

	m->presets[index] = std::move(equalizer);

	return RenameError::NoError;
}

bool Equalizer::deletePreset(int index)
{
	const auto& equalizer = equalizerSetting(index);
	const auto success = m->db->deleteEqualizer(equalizer.id());
	if(!success)
	{
		return false;
	}

	m->presets.removeAt(index);
	if(m->currentIndex >= m->presets.size())
	{
		m->currentIndex = m->presets.size() - 1;
	}

	if(m->presets.isEmpty())
	{
		auto defaultEqualizer = EqualizerSetting(-1, Lang::get(Lang::Default));
		const auto id = m->db->insertEqualizer(defaultEqualizer);
		if(id > 0)
		{
			defaultEqualizer.setId(id);
			m->presets << defaultEqualizer;
			m->currentIndex = 0;
		}
	}

	m->applyEqualizer(equalizerSetting(m->currentIndex));

	return true;
}

Equalizer::RenameError Equalizer::saveCurrentEqualizerAs(const QString& name)
{
	if(name.trimmed().isEmpty())
	{
		return RenameError::EmptyName;
	}

	bool contains = Algorithm::contains(names(), [&](const auto& str) {
		return (name.trimmed() == str.trimmed());
	});

	if(contains)
	{
		return RenameError::NameAlreadyKnown;
	}

	auto equalizer = this->currentSetting();
	equalizer.setName(name);
	equalizer.setDefaultValues(equalizer.values());

	auto id = m->db->insertEqualizer(equalizer);
	if(id >= 0)
	{
		equalizer.setId(id);
		m->presets.append(equalizer);

		return RenameError::NoError;
	}

	else
	{
		return RenameError::DbError;
	}
}

void Equalizer::setGaussEnabled(bool enabled)
{
	m->gaussEnabled = enabled;
	SetSetting(Set::Eq_Gauss, enabled);
}

bool Equalizer::isGaussEnabled() const
{
	return m->gaussEnabled;
}

const EqualizerSetting& Equalizer::equalizerSetting(int index) const
{
	return Util::between(index, m->presets) ?
	       m->presets[index] :
	       m->presets[0];
}

EqualizerSetting& Equalizer::equalizerSetting(int index)
{
	return Util::between(index, m->presets) ?
	       m->presets[index] :
	       m->presets[0];
}

const EqualizerSetting& Equalizer::currentSetting() const
{
	return equalizerSetting(m->currentIndex);
}

int Equalizer::currentIndex() const
{
	return m->currentIndex;
}

void Equalizer::setCurrentIndex(int index)
{
	index = std::max(index, 0);

	m->currentIndex = index;
	SetSetting(Set::Eq_Last, index);
}

void Equalizer::startValueChange()
{
	m->referenceValue = this->currentSetting();
}

void Equalizer::endValueChange()
{
	m->referenceValue = EqualizerSetting();
}

int Equalizer::count() const
{
	return m->presets.count();
}

QList<int>
Equalizer::Private::changeValue(int equalizerIndex, int band, int value,
                                bool affectNeighbours)
{
	if(!Util::between(equalizerIndex, this->presets))
	{
		return QList<int>();
	}

	this->presets[equalizerIndex].setValue(band, value);

	if(affectNeighbours)
	{
		const auto delta = value - this->referenceValue.value(band);

		const auto mostLeft = std::max(band - 4, 0);
		const auto mostRight =
			std::min(band + 4, static_cast<int>(this->referenceValue.values().size()));

		QList<int> affectedBands;
		for(auto neighbourBand = mostLeft; neighbourBand < mostRight; neighbourBand++)
		{
			if(neighbourBand == band)
			{
				continue;
			}

			const auto distance = std::abs(band - neighbourBand);
			const auto neighbourValue = this->referenceValue.value(neighbourBand);
			const auto dNewValue =
				neighbourValue + (delta * scaleFactors[distance]);

			changeValue(equalizerIndex, neighbourBand, int(dNewValue), false);

			affectedBands << neighbourBand;
		}

		return affectedBands;
	}

	return QList<int> {band};
}
