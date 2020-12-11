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

#include <QString>
#include <array>

/**
 * @brief The EQ_Setting class. Container for Equalizer configurations
 * @ingroup Equalizer
 */

class EqualizerSetting
{
	PIMPL(EqualizerSetting)

public:

	using ValueArray=std::array<int, 10>;

	EqualizerSetting(int id=-1, const QString& name=QString());
	EqualizerSetting(int id, const QString& name, const ValueArray& values);
	EqualizerSetting(int id, const QString& name, const ValueArray& values, const ValueArray& defaultValues);

	EqualizerSetting(const EqualizerSetting& other);
	~EqualizerSetting();

	EqualizerSetting& operator=(const EqualizerSetting& s);

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

	int id() const;
	void setId(int id);

	/**
	 * @brief get database values for setting
	 * @return
	 */
	const ValueArray& values() const;

	const ValueArray& defaultValues() const;

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

	void setDefaultValues(const ValueArray& values);

	/**
	 * @brief checks, if preset is default preset
	 * @return true if preset is default preset, false else
	 */
	bool isDefault() const;


	ValueArray::const_iterator begin() const;
	ValueArray::const_iterator end() const;
};

#endif
