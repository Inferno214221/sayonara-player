#ifndef COVERVIEWPIXMAPCACHE_H
#define COVERVIEWPIXMAPCACHE_H

#include "Utils/Pimpl.h"

using Hash=QString;
class QPixmap;

class CoverViewPixmapCache
{
	PIMPL(CoverViewPixmapCache)

public:
	CoverViewPixmapCache();
	~CoverViewPixmapCache();

	void clear();
	void set_scaling(int scaling);
	int scaling();

	bool has_pixmap(const Hash& hash) const;
	bool has_scaled_pixmap(const Hash& hash) const;

	QPixmap pixmap(const Hash& hash, bool scaled) const;
	QPixmap scaled_pixmap(const Hash& hash);
	QPixmap invalid_pixmap() const;
	void add_pixmap(const Hash& hash, const QPixmap& pm);

	bool is_outdated(const Hash& hash) const;
	void set_cache_size(int size_orig, int size_scaled);
};

#endif // COVERVIEWPIXMAPCACHE_H
