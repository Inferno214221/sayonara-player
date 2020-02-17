/* EqualizerSetting.h */

/* Copyright (C) 2011 Michael Lugmair (Lucio Carreras)
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

#ifndef EQUALIZER_SETTING_H_
#define EQUALIZER_SETTING_H_

#include "Utils/Pimpl.h"
#include "Utils/Settings/SettingConvertible.h"

#include <QString>
#include <array>

/**
 * @brief The EQ_Setting class. Container for Equalizer configurations
 * @ingroup Equalizer
 */

class EqualizerSetting :
	public SettingConvertible
{
	PIMPL(EqualizerSetting)

public:

	using ValueArray=std::array<int, 10>;

	EqualizerSetting(const QString& name=QString());
	EqualizerSetting(const QString& name, const ValueArray& values);
	EqualizerSetting(const EqualizerSetting& other);
	~EqualizerSetting() override;

	EqualizerSetting& operator=(const EqualizerSetting& s);

	/**
	 * @brief Compares the case insensitive string representation of two settings
	 * @param s other preset
	 * @return
	 */
	bool operator==(const EqualizerSetting& s) const;

	/**
	 * @brief get name of setting
	 * @return
	 */
	QString name() const;

	/**
	 * @brief set name of setting
	 * @param name
	 */
	void setName(const QString& name);

	/**
	 * @brief get database values for setting
	 * @return
	 */
	ValueArray values() const;

	/**
	 * @brief get specific value for a band idx. if idx is not valid, 0 is returned
	 * @param idx band index
	 * @return database value if idx is valid, 0 else
	 */
	int value(int idx) const;

	/**
	 * @brief set specific value for band
	 * @param idx band index
	 * @param val database formatted value
	 */
	void setValue(int idx, int val);

	/**
	 * @brief set all values for a specific index.
	 * If there are more than 10 values, list is stripped.
	 * If there are less, the list is filled with zeros
	 * @param values
	 */
	void setValues(const ValueArray& values);

	/**
	 * @brief append a value.
	 * If there are already more than 10 values, nothing happens
	 * @param val
	 */
//	void append_value(int val);

	/**
	 * @brief checks, if preset is default preset
	 * @return true if preset is default preset, false else
	 */
	bool isDefault() const;

	/**
	 * @brief checks, if the preset name belongs to a default preset
	 * @return true if preset is default preset, false else
	 */
	bool isDefaultName() const;

	/**
	 * @brief get default settings
	 * @return list of default settings
	 */
	static QList<EqualizerSetting> getDefaults();

	/**
	 * @brief get default values for a specific preset.
	 * If the preset does not have default values, an empty list is returned
	 * @param name preset name
	 * @return value list if name belongs to a default preset. Empty list else
	 */
	static ValueArray getDefaultValues(const QString& name);

	/**
	 * @brief static convenience function for is_default_name()
	 * @param name preset name
	 * @return
	 */
	static bool isDefaultName(const QString& name);

	/**
	 * @brief converts a string to a EQ_Setting.
	 * If not possible a default constructed EQ_Setting is returned
	 * @param str
	 * @return
	 */
	bool loadFromString(const QString& str) override;

	/**
	 * @brief converts EQ_Setting to string
	 * @return
	 */
	QString toString() const override;
};

#endif
