/* LocalCoverSearcher.cpp */

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

#include "LocalCoverSearcher.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QMap>

#include <cmath>

using namespace Cover;

namespace
{
	constexpr const char* ClassName = "LocalSearcher";

	double calcNameFactor(const QString& name)
	{
		if(name.contains("cover", Qt::CaseInsensitive) ||
		   name.contains("albumart", Qt::CaseInsensitive) ||
		   name.contains("front", Qt::CaseInsensitive) ||
		   name.contains("folder", Qt::CaseInsensitive))
		{
			if(name.contains("large", Qt::CaseInsensitive))
			{
				return 2.0;
			}

			else if(!name.contains("small", Qt::CaseInsensitive))
			{
				return 3.0;
			}

			else
			{
				return 4.0;
			}
		}

		return 5.0;
	}

	double calcSquareFactor(int width, int height)
	{
		const auto dWidth = static_cast<double>(width);
		const auto dHeight = static_cast<double>(height);

		const auto pixels = dWidth * dHeight;

		// calc how 'square' the cover is
		const auto r = (std::abs(dHeight - dWidth) / dWidth) + 1.0;

		return (std::pow(r, 2.0) * std::max(dHeight, dWidth)) / pixels;
	}

	double calcRating(const QPixmap& pixmap, const QString& name)
	{
		if(pixmap.width() == 0)
		{
			return -1.0;
		}

		const auto squareFactor = calcSquareFactor(pixmap.width(), pixmap.height());
		const auto nameFactor = calcNameFactor(name);

		spLog(Log::Develop, ClassName)
			<< "  Coverfile: " << name
			<< ", square factor: " << QString::number(squareFactor, 'g', 2)
			<< ", name factor: " << QString::number(nameFactor, 'g', 2);

		return squareFactor * nameFactor;
	}

	QStringList getCoverFileFilters()
	{
		auto filters = Util::imageExtensions();

		QStringList upperFilters;
		Util::Algorithm::transform(filters, upperFilters, [](const auto& filter) {
			return filter.toUpper();
		});

		filters << upperFilters;

		return filters;
	}
}


QStringList LocalSearcher::coverPathsFromPathHint(const QString& filename)
{
	spLog(Log::Develop, ClassName) << "Search for covers. Hint: " << filename;

	auto filepath = filename;
	if(!QFileInfo(filepath).isDir())
	{
		filepath = Util::File::getParentDirectory(filename);
		spLog(Log::Develop, ClassName) << filename << " is not a directory. Try using " << filepath << " instead";

		const auto info = QFileInfo(filepath);
		if(!info.isDir() || !info.exists())
		{
			return QStringList();
		}
	}

	const auto dir = QDir(filepath);
	const auto entries = dir.entryList(getCoverFileFilters(), (QDir::Files | QDir::NoDotAndDotDot));
	if(entries.isEmpty())
	{
		return QStringList();
	}

	QStringList ret;
	QMap<QString, double> sizeMap;
	for(const auto& entry : entries)
	{
		const auto pixmapPath = filepath + "/" + entry;
		const auto pixmap = QPixmap(pixmapPath);
		const auto rating = calcRating(pixmap, entry);

		spLog(Log::Develop, ClassName)
			<< "  Coverfile " << pixmapPath << " has final rating " << QString::number(rating, 'g', 2)
			<< " (Lower is better)";

		sizeMap[pixmapPath] = rating;
		ret << pixmapPath;
	}

	Util::Algorithm::sort(ret, [=](const auto& file1, const auto& file2) {
		return (sizeMap[file1] < sizeMap[file2]);
	});

	spLog(Log::Develop, ClassName) << "  Sorted cover files " << ret;

	return ret;
}
