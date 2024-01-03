/* Image.cpp */

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

#include "Image.h"
#include "Utils.h"
#include "Utils/Logger/Logger.h"

#include <QPixmap>
#include <QByteArray>

using Util::Image;

struct Image::Private
{
	QByteArray img;
};

Image::Image() :
	m(nullptr) {}

Image::Image(const QPixmap& pm) :
	Image::Image(pm, QSize(-1, -1)) {}

Image::Image(const QPixmap& pm, const QSize& max_size)
{
	m = new Private();

	if(pm.isNull())
	{
		spLog(Log::Warning, this) << "Pixmap is null!";
	}

	else
	{
		int mw = max_size.width();
		int mh = max_size.height();

		int pw = pm.width();
		int ph = pm.height();

		QPixmap p(pm);
		if(mh <= 0 || mw <= 0)
		{
			p = pm;
		}

		else if((pw > mw) || (ph > mh))
		{
			p = pm.scaled(mw, mh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		m->img = Util::convertPixmapToByteArray(p);

		if(m->img.size() == 0)
		{
			spLog(Log::Warning, this) << "Could not compress " << m->img.size() << " bytes of image data";
		}
	}
}

Image::Image(const Image& other)
{
	if(!other.m)
	{
		m = nullptr;
		return;
	}

	m = new Private();
	m->img = other.m->img;
}

Image::~Image()
{
	if(m)
	{
		delete m;
		m = nullptr;
	}
}

class Image& Image::operator=(const Image& other)
{
	if(other.m)
	{
		if(!this->m)
		{
			m = new Private();
		}

		m->img = other.m->img;
	}

	else if(m)
	{
		delete m;
		m = nullptr;
	}

	return *this;
}

QPixmap Image::pixmap() const
{
	if(!m)
	{
		spLog(Log::Warning, this) << "No data";
		return QPixmap();
	}

	QPixmap pm = Util::convertByteArrayToPixmap(m->img);
	if(pm.isNull())
	{
		spLog(Log::Warning, this) << "Pixmap is empty after decompressing (" << m->img.size() << "," << m->img.size()
		                          << " bytes)";
	}

	return pm;
}
