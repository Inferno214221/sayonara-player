/* GuiUtils.cpp */

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


/* GuiUtils.cpp */

#include "GuiUtils.h"
#include "Utils/Logger/Logger.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QDirIterator>
#include <QFontMetrics>
#include <QIcon>
#include <QList>
#include <QPalette>
#include <QPixmap>
#include <QRegExp>
#include <QScreen>
#include <QSize>
#include <QString>

using namespace Gui;

namespace
{
	QString bestResolutionPath(const QStringList& paths)
	{
		if(paths.isEmpty())
		{
			return QString();
		}

		int maxSize = 0;
		auto maxPath = paths.first();

		for(const auto& path : paths)
		{
			auto re = QRegExp(".*([0-9][0-9]+).*");
			auto idx = re.indexIn(path);
			if(idx >= 0)
			{
				int size = re.cap(1).toInt();
				if(size > maxSize)
				{
					maxSize = size;
					maxPath = path;
				}
			}
		}

		return maxPath;
	}

	QStringList iconPaths(const QString& iconName, Gui::Util::IconTheme theme)
	{
		if(iconName.trimmed().isEmpty())
		{
			return QStringList();
		}

		const auto themeName = (theme == Gui::Util::MintY)
		                       ? "MintYIcons"
		                       : QString();

		QStringList paths;
		if(theme == Gui::Util::MintY)
		{
			const auto reString = QString(".*%1/%2-[0-9]+.*")
				.arg(themeName)
				.arg(iconName);

			auto re = QRegExp(reString);

			QDirIterator it(":/" + themeName);
			while(it.hasNext())
			{
				const auto path = it.next();
				if(re.indexIn(path) >= 0)
				{
					paths << path;
				}
			}
		}

		if(paths.isEmpty())
		{
			const auto reString = QString(".*/%1(\\.[a-z][a-z][a-z])*$").arg(iconName);
			auto re = QRegExp(reString);

			QDirIterator it(":/Icons");
			while(it.hasNext())
			{
				const auto path = it.next();
				if(re.indexIn(path) >= 0)
				{
					paths << path;
				}
			}
		}

		paths.sort();
		return paths;
	}
}

QIcon Util::icon(const QString& iconName, IconTheme themeName)
{
	if(iconName.isEmpty())
	{
		return QIcon();
	}

	const auto paths = iconPaths(iconName, themeName);
	if(paths.isEmpty())
	{
		return QIcon();
	}

	QIcon icon;
	const auto path = bestResolutionPath(paths);
	if(!path.isEmpty())
	{
		const auto pm = QPixmap(path);
		if(!pm.isNull())
		{
			icon.addPixmap(pm);
		}

		else
		{
			icon.addFile(path);
		}
	}

	if(icon.isNull())
	{
		spLog(Log::Warning, "GuiUtils") << "Icon " << iconName << " does not exist";
	}

	return icon;
}

QImage Util::image(const QString& iconName, IconTheme themeName)
{
	return image(iconName, themeName, QSize(0, 0));
}

QImage Util::image(const QString& iconName, IconTheme themeName, QSize sz, bool keepAspectRatio)
{
	const auto paths = iconPaths(iconName, themeName);
	if(paths.isEmpty())
	{
		return QImage();
	}

	const auto path = bestResolutionPath(paths);
	const auto image = QImage(path);
	if(image.isNull())
	{
		spLog(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
	}

	if(sz.width() == 0)
	{
		return image;
	}

	return image.scaled(sz,
	                    keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio,
	                    Qt::SmoothTransformation);
}

QPixmap Util::pixmap(const QString& iconName, IconTheme themeName)
{
	return pixmap(iconName, themeName, QSize(0, 0));
}

QPixmap Util::pixmap(const QString& iconName, IconTheme themeName, QSize sz, bool keepAspectRatio)
{
	const auto paths = iconPaths(iconName, themeName);
	if(paths.isEmpty())
	{
		return QPixmap();
	}

	const auto path = bestResolutionPath(paths);

	const auto pixmap = QPixmap(path);
	if(pixmap.isNull())
	{
		spLog(Log::Warning, "GuiUtils") << "Pixmap " << paths.first() << " does not exist";
	}

	if(sz.width() == 0)
	{
		return pixmap;
	}

	return pixmap.scaled(sz,
	                     keepAspectRatio ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio,
	                     Qt::SmoothTransformation);
}

QColor Util::color(QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole)
{
	return QApplication::palette().color(colorGroup, colorRole);
}

QScreen* Util::getBiggestScreen()
{
	const auto screens = QApplication::screens();
	auto it = std::max_element(screens.begin(), screens.end(), [](QScreen* s1, QScreen* s2) {
		return (s1->size().height() < s2->size().height());
	});

	if(it != screens.end())
	{
		return *it;
	}

	return (!screens.isEmpty())
	       ? screens.first()
	       : nullptr;
}

void Util::placeInScreenCenter(QWidget* widget, float relativeSizeX, float relativeSizeY)
{
	auto* screen = getBiggestScreen();
	if(!screen)
	{
		return;
	}

	const auto width = screen->size().width();
	const auto height = screen->size().height();

	if(relativeSizeX < 0.1f || relativeSizeY < 0.1f)
	{
		relativeSizeX = 0.7f;
		relativeSizeY = 0.7f;
	}

	const auto xRemainder = (1.0f - relativeSizeX) / 2.0f;
	const auto yRemainder = (1.0f - relativeSizeY) / 2.0f;

	const auto xAbs = std::max(0, static_cast<int>(width * xRemainder)) + screen->geometry().x();
	const auto yAbs = std::max(0, static_cast<int>(height * yRemainder)) + screen->geometry().y();
	auto wAbs = std::max(0, static_cast<int>(width * relativeSizeX));
	auto hAbs = std::max(0, static_cast<int>(height * relativeSizeY));

	if(wAbs == 0)
	{
		wAbs = 1200;
	}

	if(hAbs == 0)
	{
		hAbs = 800;
	}

	widget->setGeometry(xAbs, yAbs, wAbs, hAbs);
}

int Util::textWidth(const QFontMetrics& fm, const QString& text)
{
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 11
	return fm.horizontalAdvance(text);
#else
	return fm.width(text);
#endif
}

int Util::textWidth(QWidget* widget, const QString& text)
{
	return textWidth(widget->fontMetrics(), text);
}

int Util::viewRowHeight()
{
	return -1;
}
