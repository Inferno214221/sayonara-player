/* LocalCoverSearcher.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QMap>

#include <cmath>

using namespace Cover;
static const char* ClassName="LocalSearcher";

static double calc_name_factor(const QString& name)
{
	if( name.contains(QStringLiteral("cover"), Qt::CaseInsensitive) ||
		name.contains(QStringLiteral("albumart"), Qt::CaseInsensitive) ||
		name.contains(QStringLiteral("front"), Qt::CaseInsensitive) ||
		name.contains(QStringLiteral("folder"), Qt::CaseInsensitive))
	{
		if(name.contains(QStringLiteral("large"), Qt::CaseInsensitive)){
			return 2.0;
		}

		else if(!name.contains(QStringLiteral("small"), Qt::CaseInsensitive)){
			return 3.0;
		}

		else {
			return 4.0;
		}
	}

	return 5.0;
}

static double calc_square_factor(int width, int height)
{

	double d_width = double(width);
	double d_height = double(height);

	double pixels = d_width * d_height;

	// calc how 'square' the cover is
	double r = (std::abs(d_height - d_width) / d_width) + 1.0;
	return (r * r * std::max(d_height, d_width)) / pixels;
}

static double calc_rating(const QPixmap& pixmap, const QString& name)
{
	if(pixmap.width() == 0){
		return -1.0;
	}

	double square_factor = calc_square_factor(pixmap.width(), pixmap.height());
	double name_factor = calc_name_factor(name);

	sp_log(Log::Develop, ClassName) << "  Coverfile " << name << " has square factor " << QString::number(square_factor, 'g', 2);
	sp_log(Log::Develop, ClassName) << "  Coverfile " << name << " has name factor " << QString::number(name_factor, 'g', 2);

	return square_factor * name_factor;
}

QStringList LocalSearcher::cover_paths_from_path_hint(const QString& filepath_hint)
{
	sp_log(Log::Develop, ClassName) << "Search for covers. Hint: " << filepath_hint;

	QString filepath = filepath_hint;
	QFileInfo info(filepath);
	if(!info.isDir())
	{
		filepath = Util::File::get_parent_directory(filepath_hint);
		sp_log(Log::Develop, ClassName) << filepath_hint << " is not a directory. Try using " << filepath << " instead";

		info = QFileInfo(filepath);
		if(!info.isDir() || !info.exists())	{
			return QStringList();
		}
	}

	QStringList filters = Util::image_extensions();
	QStringList upper_filters;

	Util::Algorithm::transform(filters, upper_filters, [](const QString& filter){
		return filter.toUpper();
	});

	filters << upper_filters;

	const QDir dir(filepath);
	const QStringList entries = dir.entryList(filters, (QDir::Files | QDir::NoDotAndDotDot));
	if(entries.isEmpty()){
		return QStringList();
	}

	QStringList ret;
	QMap<QString, double> size_map;
	for(const QString& entry : entries)
	{
		QString f = filepath + "/" + entry;

		QPixmap pm(f + "/" + entry);
		double rating = calc_rating(pm, entry);

		sp_log(Log::Develop, ClassName) << "  Coverfile " << f << " has final rating " << QString::number(rating, 'g', 2)  << " (Lower is better)";

		size_map[f] = rating;
		ret << f;
	}

	Util::Algorithm::sort(ret, [=](const QString& f1, const QString& f2)
	{
		return (size_map[f1] < size_map[f2]);
	});

	sp_log(Log::Develop, ClassName) << "  Sorted cover files " << ret;

	return ret;
}

