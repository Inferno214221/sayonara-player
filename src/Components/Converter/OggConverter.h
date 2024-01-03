/* OggConverter.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef OGG_CONVERTER_H
#define OGG_CONVERTER_H

#include "Converter.h"

class MetaDataList;
class MetaData;

class OggConverter :
	public Converter
{
	Q_OBJECT

	public:
		OggConverter(int quality, QObject* parent = nullptr);
		~OggConverter() override;

		QStringList supportedInputFormats() const override;

	protected:
		QString binary() const override;
		QStringList processEntry(const MetaData& md) const override;
		QString extension() const override;
};

#endif // OGGCONVERTER_H
