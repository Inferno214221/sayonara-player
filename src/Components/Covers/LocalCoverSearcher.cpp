/* LocalCoverSearcher.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QMap>

#include <cmath>

using namespace Cover;
static const char* ClassName="LocalSearcher";

QStringList LocalSearcher::cover_paths_from_path_hint(const QString& filepath_hint)
{
	sp_log(Log::Develop, ClassName) << "Search for covers. Hint: " << filepath_hint;

	QString filepath = filepath_hint;
	QFileInfo info(filepath);
	if(info.isFile())
	{
		filepath = Util::File::get_parent_directory(filepath_hint);
		sp_log(Log::Develop, ClassName) << filepath_hint << " is not a directory. Using " << filepath << " instead";
	}

	QStringList filters = Util::image_extensions();

	int c = filters.size();
	for(int i=0; i<c; i++){
		filters << filters[i].toUpper();
	}

	QDir dir(filepath);
	QStringList entries = dir.entryList(filters, (QDir::Files | QDir::NoDotAndDotDot));
	if(entries.isEmpty()){
		return QStringList();
	}

	QStringList ret;
	QMap<QString, double> size_map;
	for(const QString& entry : entries)
	{
		QImage i(filepath + "/" + entry);
		if(i.isNull()){
			continue;
		}

		int width = i.width();
		int height = i.height();
		if(width == 0){
			continue;
		}

		double d = std::abs(height - width) / (width * 1.0) + 1.0;
		double pixels = double(width * height);

		d = (d * d * std::max(width, height)) / pixels;

		sp_log(Log::Develop, ClassName) << "  Coverfile " << filepath + "/" + entry << " has first rating " << QString::number(d, 'g', 2);

		if( entry.contains(QStringLiteral("cover"), Qt::CaseInsensitive) ||
			entry.contains(QStringLiteral("albumart"), Qt::CaseInsensitive) ||
			entry.contains(QStringLiteral("front"), Qt::CaseInsensitive) ||
			entry.contains(QStringLiteral("folder"), Qt::CaseInsensitive))
		{
			if(entry.contains(QStringLiteral("large"), Qt::CaseInsensitive)){
				d = d * 2;
			}

			else if(!entry.contains(QStringLiteral("small"), Qt::CaseInsensitive)){
				d = d * 3;
			}

			else {
				d = d * 4;
			}
		}

		else
		{
			d = d * 5;
		}

		sp_log(Log::Develop, ClassName) << "  Coverfile " << filepath + "/" + entry << " has final rating " << QString::number(d, 'g', 2)  << " (Lower is better)";

		QString f = filepath + "/" + entry;
		size_map[f] = d;
		ret << f;
	}

	std::sort(ret.begin(), ret.end(), [=](const QString& f1, const QString& f2){
		return (size_map[f1] < size_map[f2]);
	});

	sp_log(Log::Develop, ClassName) << "  Sorted cover files " << ret;

	return ret;
}

