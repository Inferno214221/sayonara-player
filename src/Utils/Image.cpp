#include "Image.h"
#include "Utils.h"
#include "Compressor/Compressor.h"
#include "Utils/Logger/Logger.h"

#include <QPixmap>
#include <QByteArray>

struct Util::Image::Private
{
	QByteArray img;
};

Util::Image::Image() :
	m(nullptr)
{}

Util::Image::Image(const QPixmap& pm) :
	Util::Image(pm, QSize(-1, -1))
{}

Util::Image::Image(const QPixmap& pm, const QSize& max_size)
{
	m = new Private();

	if(pm.isNull()){
		sp_log(Log::Warning, this) << "Pixmap is null!";
	}

	else
	{
		int mw = max_size.width();
		int mh = max_size.height();

		int pw = pm.width();
		int ph = pm.height();

		QPixmap p(pm);
		if(mh <= 0 || mw <= 0){
			p = pm;
		}

		else if((pw > mw) || (ph > mh)){
			p = pm.scaled(mw, mh, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		QByteArray arr = Util::cvt_pixmap_to_bytearray(p);
		m->img = Compressor::compress(arr);

		/*sp_log(Log::Debug, this) << "Compressed image (" << pw << "x" << ph << ") from "
								 << tmp_arr.size() << " to " << m->img.size()
								 << ": " << m->img.size() * 100.0 / tmp_arr.size() << "%";*/

		if(m->img.size() == 0){
			sp_log(Log::Warning, this) << "Could not compress " << arr.size() << " bytes of image data";
		}
	}
}

Util::Image::Image(const Util::Image& other)
{
	if(!other.m){
		return;
	}

	m = new Private();
	m->img = other.m->img;
}


Util::Image::~Image()
{
	if(m)
	{
		delete m; m=nullptr;
	}
}

Util::Image& Util::Image::operator=(const Util::Image& other)
{
	if(other.m)
	{
		if(!this->m)
		{
			m = new Private();
		}

		m->img = other.m->img;
	}

	else if(m){
		delete m; m=nullptr;
	}

	return *this;
}

QPixmap Util::Image::pixmap() const
{
	if(!m){
		sp_log(Log::Warning, this) << "No data";
		return QPixmap();
	}

	QByteArray decompressed = Compressor::decompress(m->img);
	QPixmap pm = Util::cvt_bytearray_to_pixmap(decompressed);
	if(pm.isNull()){
		sp_log(Log::Warning, this) << "Pixmap is empty after decompressing (" << decompressed.size() << "," << m->img.size() << " bytes)";
	}

	return pm;
}
