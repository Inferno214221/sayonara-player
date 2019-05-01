/* CoverViewPixmapCache.h */

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



#ifndef COVERVIEWPIXMAPCACHE_H
#define COVERVIEWPIXMAPCACHE_H

#include "Utils/Pimpl.h"
#include <QThread>

using Hash=QString;
class QPixmap;

class CoverViewPixmapCache : public QThread
{
	Q_OBJECT
	PIMPL(CoverViewPixmapCache)

signals:
	void sig_hash_ready(const QString& hash);

public:
	CoverViewPixmapCache(QObject* parent=nullptr);
	~CoverViewPixmapCache();

	void clear();
	void set_scaling(int scaling);
	int scaling();

	bool has_pixmap(const Hash& hash) const;
	bool has_scaled_pixmap(const Hash& hash) const;
	bool is_in_queue(const Hash& hash) const;

	QPixmap pixmap(const Hash& hash, bool scaled) const;
	QPixmap scaled_pixmap(const Hash& hash);
	QPixmap invalid_pixmap() const;
	void	add_pixmap(const Hash& hash, const QPixmap& pm);

	bool is_outdated(const Hash& hash) const;
	void set_cache_size(int size_orig, int size_scaled);

protected:
	void run() override;
};

#endif // COVERVIEWPIXMAPCACHE_H
