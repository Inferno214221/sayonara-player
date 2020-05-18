#include "ImageButton.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Gui/Utils/Icons.h"

#include <QPixmap>
#include <QTimer>
#include <QPainter>
#include <QThread>
#include <QMouseEvent>

using Gui::ImageButton;
using Gui::ByteArrayConverter;

struct ByteArrayConverter::Private
{
	QByteArray	data;
	QString		mime;
	QPixmap		pixmap;

	Private(const QByteArray& data, const QString& mime) :
		data(data),
		mime(mime)
	{}
};

ByteArrayConverter::ByteArrayConverter(const QByteArray& data, const QString& mime) :
	QObject()
{
	m = Pimpl::make<Private>(data, mime);
}

ByteArrayConverter::~ByteArrayConverter() = default;

QPixmap ByteArrayConverter::pixmap() const
{
	return m->pixmap;
}

void ByteArrayConverter::start()
{
	m->pixmap.loadFromData(m->data, m->mime.toLocal8Bit().data());
	emit sigFinished();
}


struct ImageButton::Private
{
	QByteArray currentHash;
	QByteArray oldHash;

	QPixmap currentPixmap, currentPixmapScaled;
	QPixmap oldPixmap, oldPixmapScaled;
	QPixmap invalidPixmap;

	QRect pixmapRect;

	double opacity;

	QTimer* timer=nullptr;
	bool isFadingEnabled;

	Private() :
		invalidPixmap(QPixmap("://Icons/logo.png")),
		opacity(1.0),
		isFadingEnabled(true)
	{
		this->currentPixmap = this->invalidPixmap;
	}
};

ImageButton::ImageButton(QWidget* parent) :
	QPushButton (parent)
{
	m = Pimpl::make<Private>();

	m->timer = new QTimer(this);
	m->timer->setTimerType(Qt::TimerType::PreciseTimer);
	m->timer->setInterval(10);

	this->setFlat(true);
	this->setMouseTracking(true);

	connect(m->timer, &QTimer::timeout, this, &ImageButton::timerTimedOut);
}

ImageButton::~ImageButton()
{
	if(m->timer->isActive()){
		m->timer->stop();
	}
}

void ImageButton::setPixmap(const QPixmap& _pm)
{
	QPixmap pm(_pm);
	if(pm.isNull()) {
		pm = m->invalidPixmap;
	}

	{ // check if the image is already the same
		QPixmap pmScaled50 = pm.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation);
		auto newHash = Util::calcHash(Util::convertPixmapToByteArray(pmScaled50));
		if(newHash == m->currentHash) {
			return;
		}

		m->currentHash = newHash;
	}

	// don't change the currently fading out cover
	if(!m->timer->isActive())
	{
		m->oldPixmap = m->currentPixmap;
		m->oldPixmapScaled = m->currentPixmapScaled;
	}

	m->currentPixmap = pm;
	m->currentPixmapScaled = m->currentPixmap.scaled
	(
		(this->size() - QSize(2, 2)),
		Qt::KeepAspectRatio,
		Qt::SmoothTransformation
	);

	this->setToolTip
	(
		QString("%1x%2")
			.arg(pm.width())
			.arg(pm.height())
	);

	if(m->isFadingEnabled)
	{
		m->opacity = 0;
		m->timer->start();
	}

	else
	{
		m->timer->stop();
		m->opacity = 1.0;
		repaint();
	}

	emit sigPixmapChanged();
}

void ImageButton::setPixmapPath(const QString& path)
{
	setPixmap(QPixmap(path));
}

QPixmap ImageButton::pixmap() const
{
	return m->currentPixmap;
}

void ImageButton::setCoverData(const QByteArray& data, const QString& mimetype)
{
	auto* thread = new QThread();
	auto* worker = new ByteArrayConverter(data, mimetype);
	worker->moveToThread(thread);

	connect(worker, &ByteArrayConverter::sigFinished, this, &ImageButton::byteconverterFinished);
	connect(worker, &ByteArrayConverter::sigFinished, thread, &QThread::quit);
	connect(thread, &QThread::started, worker, &ByteArrayConverter::start);
	connect(thread, &QThread::finished, thread, &QObject::deleteLater);

	thread->start();
}

void ImageButton::showDefaultPixmap()
{
	this->setPixmap(m->invalidPixmap);
}

void ImageButton::byteconverterFinished()
{
	auto* worker = static_cast<ByteArrayConverter*>(sender());

	const QPixmap pm = worker->pixmap();
	if(!pm.isNull()) {
		this->setPixmap(pm);
	}

	else {
		spLog(Log::Warning, this) << "Cover from track seems invalid or broken";
	}

	worker->deleteLater();
}

int ImageButton::verticalPadding() const
{
	if(m->currentPixmapScaled.isNull())
	{
		// if this is not handled, the padding is always
		// approx the button height. But that should never
		// happen as the invalid cover should be set when
		// possible
		return 0;
	}

	int p = (this->height() - m->currentPixmapScaled.size().height()) - 2;
	if(p <= 0){
		p = -(this->width() - m->currentPixmapScaled.size().width() - 2);
	}

	return p;
}

void ImageButton::setFadingEnabled(bool b)
{
	m->isFadingEnabled = b;
}

void ImageButton::timerTimedOut()
{
	m->opacity = std::min(1.0, m->opacity + 0.025);

	if(m->opacity < 1.0)
	{
		repaint();
	}

	else
	{
		m->oldPixmap = QPixmap();
		m->oldPixmapScaled = QPixmap();
		m->timer->stop();
	}
}

void ImageButton::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e)

	if(m->currentPixmapScaled.isNull()){
		return;
	}

	QPainter painter(this);

	int h = this->height() - 2;
	int w = this->width() - 2;

	QPixmap pm = m->currentPixmapScaled;
	QPixmap pmOld;
	if(!m->oldPixmapScaled.isNull())
	{
		pmOld = m->oldPixmapScaled;
	}

	int x = (w - pm.width()) / 2;
	int y = (h - pm.height()) / 2;

	if(!pmOld.isNull())
	{
		int xOld = (w - pmOld.width()) / 2;
		int yOld = (h - pmOld.height()) / 2;

		painter.setOpacity(1.0 - m->opacity);
		painter.drawPixmap
		(
			xOld, yOld, pmOld.width(), pmOld.height(),
			pmOld
		);

		m->pixmapRect = QRect(xOld, yOld, pmOld.width(), pmOld.height());

		painter.setOpacity(m->opacity);
	}

	else {
		painter.setOpacity(1.0);
	}

	if(!pm.isNull())
	{
		painter.drawPixmap
		(
			x, y, pm.width(), pm.height(),
			pm
		);

		m->pixmapRect = QRect(x, y, pm.width(), pm.height());
	}
}

void ImageButton::resizeEvent(QResizeEvent* e)
{
	QPushButton::resizeEvent(e);

	int h = this->height() - 2;
	int w = this->width() - 2;

	if(m->currentPixmap.isNull()){
		return;
	}

	m->currentPixmapScaled = m->currentPixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ImageButton::mouseMoveEvent(QMouseEvent* e)
{
	bool within = m->pixmapRect.contains(e->pos());

	auto shape = (within) ? Qt::PointingHandCursor : Qt::ArrowCursor;
	this->setCursor(QCursor(shape));

	QPushButton::mouseMoveEvent(e);
}

void ImageButton::mouseReleaseEvent(QMouseEvent* e)
{
	if(e->button() | Qt::LeftButton)
	{
		bool within = m->pixmapRect.contains(e->pos());
		if(!within)
		{
			QPushButton::mouseReleaseEvent(e);
			return;
		}

		emit sigTriggered();
	}

	QPushButton::mouseReleaseEvent(e);
}
