#include "CoverViewPixmapCache.h"
#include "Components/Covers/CoverLocation.h"
#include "Utils/Image.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Set.h"

#include <QCache>
#include <QString>
#include <QPixmap>

struct CoverViewPixmapCache::Private
{
	QCache<Hash, Util::Image>	pixmaps;
	QCache<Hash, Util::Image>	scaled_pixmaps;
	Util::Set<Hash>				valid_hashes;

	QPixmap						invalid_cover;
	int							scaling;

	Private(int scaling) :
		scaling(scaling)
	{
		pixmaps.setMaxCost(50);
		scaled_pixmaps.setMaxCost(50);
	}
};

static QPixmap scale_pixmap(const QPixmap& pm, int w, int h)
{
	return pm.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

CoverViewPixmapCache::CoverViewPixmapCache()
{
	int scaling = Settings::instance()->get<Set::Lib_CoverZoom>();
	m = Pimpl::make<Private>(scaling);
}

CoverViewPixmapCache::~CoverViewPixmapCache() {}

void CoverViewPixmapCache::clear()
{
	m->pixmaps.clear();
	m->scaled_pixmaps.clear();
	m->valid_hashes.clear();
}

void CoverViewPixmapCache::set_scaling(int scaling)
{
	m->scaling = scaling;
	m->scaled_pixmaps.clear();

	QString invalid_path = Cover::Location::invalid_location().cover_path();
	m->invalid_cover = scale_pixmap(QPixmap(invalid_path), m->scaling, m->scaling);
}

int CoverViewPixmapCache::scaling()
{
	return m->scaling;
}

bool CoverViewPixmapCache::has_pixmap(const Hash& hash) const
{
	return m->pixmaps.contains(hash) && (m->pixmaps.object(hash) != nullptr);
}

bool CoverViewPixmapCache::has_scaled_pixmap(const Hash& hash) const
{
	return m->scaled_pixmaps.contains(hash) && (m->scaled_pixmaps.object(hash) != nullptr);
}

QPixmap CoverViewPixmapCache::pixmap(const Hash& hash, bool zoomed) const
{
	Util::Image* img = m->pixmaps.object(hash);
	if(!img){
		return QPixmap();
	}

	QPixmap pm = img->pixmap();
	if(zoomed){
		pm = scale_pixmap(pm, m->scaling, m->scaling);
	}

	return pm;
}

QPixmap CoverViewPixmapCache::invalid_pixmap() const
{
	return m->invalid_cover;
}

QPixmap CoverViewPixmapCache::scaled_pixmap(const Hash& hash)
{
	if(!has_scaled_pixmap(hash))
	{
		if(!has_pixmap(hash)) {
			return m->invalid_cover;
		}

		else {
			QPixmap pm = this->pixmap(hash, true);

			m->scaled_pixmaps.insert(hash, new Util::Image(pm, QSize(m->scaling, m->scaling)));
			return pm;
		}
	}

	return m->scaled_pixmaps.object(hash)->pixmap();
}

void CoverViewPixmapCache::add_pixmap(const Hash& hash, const QPixmap& pm)
{
	if(pm.isNull()){
		return;
	}

	m->pixmaps.insert(hash, new Util::Image(pm, QSize(200, 200)));
	m->scaled_pixmaps.insert(hash, new Util::Image(pm, QSize(m->scaling, m->scaling)));
	m->valid_hashes.insert(hash);
}

bool CoverViewPixmapCache::is_outdated(const Hash& hash) const
{
	return (!m->valid_hashes.contains(hash));
}

void CoverViewPixmapCache::set_cache_size(int size_orig, int size_scaled)
{
	m->scaled_pixmaps.setMaxCost(size_scaled);
	m->pixmaps.setMaxCost(size_orig);
}
