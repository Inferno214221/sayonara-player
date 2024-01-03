/* DiscnumberFrame.cpp */

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

#include "DiscnumberFrame.h"

#include <QStringList>

#include <optional>

namespace
{
	std::optional<Models::Discnumber> discnumberFromQString(const QString& data)
	{
		const auto splittedData = data.split("/");

		auto discnumber = Models::Discnumber();
		if(splittedData.isEmpty())
		{
			return std::optional<Models::Discnumber>();
		}

		if(splittedData.size() > 0)
		{
			discnumber.disc = splittedData[0].toInt();
		}

		if(splittedData.size() > 1)
		{
			discnumber.disccount = splittedData[1].toInt();
		}

		return std::optional(discnumber);
	}
}

Xiph::DiscnumberFrame::DiscnumberFrame(TagLib::Ogg::XiphComment* tag) :
	Xiph::XiphFrame<Models::Discnumber>(tag, "DISCNUMBER") {}

Xiph::DiscnumberFrame::~DiscnumberFrame() = default;

std::optional<Models::Discnumber> Xiph::DiscnumberFrame::mapTagToData() const
{
	const auto data = stringData();

	return (data.has_value())
	       ? discnumberFromQString(Tagging::convertString(data.value()))
	       : std::nullopt;
}

void Xiph::DiscnumberFrame::mapDataToTag(const Models::Discnumber& model)
{
	const auto value = QString("%1/%2")
		.arg(static_cast<uint32_t>(model.disc))
		.arg(static_cast<uint32_t>(model.disccount));

	this->setStringData(Tagging::convertString(value));
}
