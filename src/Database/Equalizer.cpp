/* Equalizer.cpp */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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
#include "Utils/EqualizerSetting.h"
#include "Utils/Algorithm.h"
#include "Query.h"

namespace Algorithm = Util::Algorithm;
using DB::Equalizer;

namespace
{
	constexpr const auto TableName = "Equalizer";

	EqualizerSetting::ValueArray convertVariantToValues(const QVariant& data)
	{
		EqualizerSetting::ValueArray values;
		const auto list = data.toString().split(',');

		for(auto i = 0; (i < int(values.size()) && i < list.size()); i++)
		{
			values[i] = list[i].toInt();
		}

		return values;
	}

	QVariant convertValuesToVariant(const EqualizerSetting::ValueArray& data)
	{
		QStringList stringList;
		Algorithm::transform(data, stringList, [](const auto value) {
			return QString::number(value);
		});

		return QVariant::fromValue(stringList.join(','));
	}
}

Equalizer::Equalizer(const QString& connectionName, DbId databaseId) :
	DB::Module(connectionName, databaseId) {}

Equalizer::~Equalizer() = default;

bool DB::Equalizer::deleteEqualizer(int id)
{
	const auto q = runQuery("DELETE from Equalizer WHERE id=:id;",
	                        {":id", id},
	                        "Cannot remove equalizer");

	return !hasError(q);
}

int DB::Equalizer::insertEqualizer(const EqualizerSetting& equalizer)
{
	const auto q =
		insert(TableName,
		       {{"name",            equalizer.name()},
		        {"equalizerValues", convertValuesToVariant(equalizer.values())},
		        {"defaultValues",   convertValuesToVariant(equalizer.defaultValues())}
		       },
		       "Cannot insert equalizer");

	return hasError(q)
	       ? -1
	       : q.lastInsertId().toInt();
}

bool DB::Equalizer::updateEqualizer(const EqualizerSetting& equalizer)
{
	const auto q =
		update(TableName,
		       {{"name",            equalizer.name()},
		        {"equalizerValues", convertValuesToVariant(equalizer.values())},
		        {"defaultValues",   convertValuesToVariant(equalizer.defaultValues())}},
		       {"id", equalizer.id()},
		       "Cannot update equalizer");

	return wasUpdateSuccessful(q);
}

bool DB::Equalizer::fetchAllEqualizers(QList<EqualizerSetting>& equalizers)
{
	auto q = runQuery(
		"SELECT id, name, equalizerValues, defaultValues FROM Equalizer;",
		"Cannot fetch equalizers");

	if(hasError(q))
	{
		return false;
	}

	equalizers.clear();
	while(q.next())
	{
		auto id = q.value(0).toInt();
		auto name = q.value(1).toString();
		auto values = convertVariantToValues(q.value(2));
		auto defaultValues = convertVariantToValues(q.value(3));

		EqualizerSetting eq(id, name, values, defaultValues);
		equalizers.push_back(std::move(eq));
	}

	return true;
}

QList<EqualizerSetting> DB::Equalizer::factoryDefaults()
{
	return QList<EqualizerSetting> {
		EqualizerSetting(-1, "Flat", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
		EqualizerSetting(-1, "Rock", {2, 4, 8, 3, 1, 3, 7, 10, 14, 14}),
		EqualizerSetting(-1, "Light Rock", {1, 1, 2, 1, -2, -3, -1, 3, 5, 8}),
		EqualizerSetting(-1, "Treble", {0, 0, -3, -5, -3, 2, 8, 15, 17, 13}),
		EqualizerSetting(-1, "Bass", {13, 17, 15, 8, 2, -3, -5, -3, 0, 0}),
		EqualizerSetting(-1, "Mid", {0, 0, 1, 5, 8, 14, 7, 4, 0, 0})
	};
}

bool DB::Equalizer::restoreFactoryDefaults()
{
	QList<EqualizerSetting> equalizers;
	fetchAllEqualizers(equalizers);

	for(const auto& equalizer: equalizers)
	{
		deleteEqualizer(equalizer.id());
	}

	const auto defaults = DB::Equalizer::factoryDefaults();
	return std::all_of(defaults.begin(), defaults.end(), [this](const auto& eq) {
		return insertEqualizer(eq);
	});
}
