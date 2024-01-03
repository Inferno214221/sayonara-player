/* ImageButton.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

constexpr const auto Padding = 2;

namespace
{
	QRect getPaintArea(const QWidget* widget)
	{
		return QRect(
			Padding, Padding,
			widget->width() - (2 * Padding), widget->height() - (2 * Padding)
		);
	}

	QRect getTargetRect(const QRect& paintArea, const QSize& pixmapSize)
	{
		return QRect(
			paintArea.x() + (paintArea.width() - pixmapSize.width()) / 2,
			paintArea.y() + (paintArea.height() - pixmapSize.height()) / 2,
			pixmapSize.width(),
			pixmapSize.height()
		);
	}

	QPixmap scalePixmap(const QPixmap& pixmap, const QWidget* widget)
	{
		const auto paintArea = getPaintArea(widget);
		return pixmap.scaled
			(
				paintArea.size(),
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation
			);
	}

	QByteArray calcHash(const QPixmap& pixmap)
	{
		const auto pmScaled50 = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::FastTransformation);
		return Util::calcHash(Util::convertPixmapToByteArray(pmScaled50));
	}
}

struct ByteArrayConverter::Private
{
	QByteArray data;
	QString mime;
	QPixmap pixmap;

	Private(const QByteArray& data, const QString& mime) :
		data(data),
		mime(mime) {}
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
	QTimer* timer = nullptr;

	QByteArray currentHash;

	QPixmap currentPixmap {":/Icons/logo.png"};
	QPixmap currentPixmapScaled;
	QPixmap oldPixmapScaled;
	QPixmap invalidPixmap {":/Icons/logo.png"};

	QRect pixmapRect;

	double opacity {1.0};
	bool isFadingEnabled {true};

	Private(ImageButton* parent) :
		timer {new QTimer(parent)}
	{
		timer->setTimerType(Qt::TimerType::PreciseTimer);
		timer->setInterval(10);
	}
};

ImageButton::ImageButton(QWidget* parent) :
	QPushButton(parent)
{
	m = Pimpl::make<Private>(this);

	this->setFlat(true);
	this->setMouseTracking(true);

	connect(m->timer, &QTimer::timeout, this, &ImageButton::timerTimedOut);
}

ImageButton::~ImageButton()
{
	if(m->timer->isActive())
	{
		m->timer->stop();
	}
}

void ImageButton::setPixmap(const QPixmap& newPixmap)
{
	const auto pixmap = (!newPixmap.isNull())
	                    ? newPixmap
	                    : m->invalidPixmap;

	const auto hash = calcHash(pixmap);
	if(hash == m->currentHash)
	{
		return;
	}

	m->currentHash = hash;

	// don't change the currently fading out cover
	if(!m->timer->isActive())
	{
		m->oldPixmapScaled = m->currentPixmapScaled;
	}

	m->currentPixmap = pixmap;
	m->currentPixmapScaled = scalePixmap(m->currentPixmap, this);

	this->setToolTip(
		QString("%1x%2")
			.arg(pixmap.width())
			.arg(pixmap.height())
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
	auto* worker = dynamic_cast<ByteArrayConverter*>(sender());

	const auto pixmap = worker->pixmap();
	if(!pixmap.isNull())
	{
		this->setPixmap(pixmap);
	}

	else
	{
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

	const auto paintArea = getPaintArea(this);
	const auto p = (paintArea.height() - m->currentPixmapScaled.height());
	return (p > 0)
	       ? p
	       : (m->currentPixmapScaled.width() - paintArea.width());
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
		m->oldPixmapScaled = QPixmap();
		m->timer->stop();
	}
}

void ImageButton::paintEvent([[maybe_unused]] QPaintEvent* e)
{
	if(m->currentPixmapScaled.isNull())
	{
		return;
	}

	QPainter painter(this);

	const auto paintArea = getPaintArea(this);

	const auto scaledPixmap = m->currentPixmapScaled;
	const auto oldScaledPixmap = (!m->oldPixmapScaled.isNull())
	                             ? m->oldPixmapScaled
	                             : QPixmap();

	if(!oldScaledPixmap.isNull())
	{
		const auto targetRect = getTargetRect(paintArea, oldScaledPixmap.size());

		painter.setOpacity(1.0 - m->opacity);
		painter.drawPixmap(targetRect, oldScaledPixmap);

		m->pixmapRect = targetRect;

		painter.setOpacity(m->opacity);
	}

	else
	{
		painter.setOpacity(1.0);
	}

	if(!scaledPixmap.isNull())
	{
		m->pixmapRect = getTargetRect(paintArea, scaledPixmap.size());
		painter.drawPixmap(m->pixmapRect, scaledPixmap);
	}
}

void ImageButton::resizeEvent(QResizeEvent* e)
{
	QPushButton::resizeEvent(e);

	if(m->currentPixmap.isNull())
	{
		return;
	}

	m->currentPixmapScaled = scalePixmap(m->currentPixmap, this);
}

void ImageButton::mouseMoveEvent(QMouseEvent* e)
{
	const auto isCursorWithin = m->pixmapRect.contains(e->pos());
	const auto shape = (isCursorWithin) ?
	                   Qt::PointingHandCursor :
	                   Qt::ArrowCursor;

	this->setCursor(QCursor(shape));

	QPushButton::mouseMoveEvent(e);
}

void ImageButton::mouseReleaseEvent(QMouseEvent* e)
{
	if(e->button() | Qt::LeftButton)
	{
		const auto isCursorWithin = m->pixmapRect.contains(e->pos());
		if(!isCursorWithin)
		{
			QPushButton::mouseReleaseEvent(e);
			return;
		}

		emit sigTriggered();
	}

	QPushButton::mouseReleaseEvent(e);
}
