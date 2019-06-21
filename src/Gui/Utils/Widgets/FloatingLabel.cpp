#include "FloatingLabel.h"
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>

using Gui::FloatingLabel;

const static int timerInterval=20;

struct FloatingLabel::Private
{
	double		offset;
	double		offset_direction;
	int			chars_per_second;
	QTimer*		timer=nullptr;
	QString		text;

	bool		floating;

	Private() :
		offset(0),
		offset_direction(-1.0),
		chars_per_second(3),
		floating(false)
	{}
};

FloatingLabel::FloatingLabel(QWidget* parent) :
	Gui::WidgetTemplate<QLabel>(parent)
{
	m = Pimpl::make<Private>();
	m->timer = new QTimer(this);
	connect(m->timer, &QTimer::timeout, this, &FloatingLabel::updateOffset);
}

FloatingLabel::~FloatingLabel() = default;

void FloatingLabel::paintEvent(QPaintEvent* event)
{
	if(!m->floating)
	{
		QLabel::paintEvent(event);
		return;
	}

	event->ignore();

	QFontMetrics fm = this->fontMetrics();
	QPainter painter(this);

	painter.drawText(
		QRectF(static_cast<int>(m->offset), 0, fm.width(m->text), fm.height()),
		m->text
	);
}

void FloatingLabel::resizeEvent(QResizeEvent* event)
{
	QLabel::resizeEvent(event);

	QFontMetrics fm = this->fontMetrics();
	int difference = fm.width(m->text) - this->width();

	if(difference <= 0)
	{
		m->timer->stop();
	}

	else if(!m->timer->isActive()){
		m->timer->start(timerInterval);
	}
}

void FloatingLabel::setFloatingText(const QString& text)
{
	QLabel::setText(text);
	m->text = text;
	m->offset = 0;
	m->offset_direction = -1.0;
	m->floating = false;

	updateOffset();
}

void FloatingLabel::setCharsPerSecond(int chars_per_second)
{
	m->chars_per_second = chars_per_second;
}

void FloatingLabel::updateOffset()
{
	static const int tolerance = 10;

	QFontMetrics fm = this->fontMetrics();
	int difference = fm.width(m->text) - this->width();

	if(difference <= 0)
	{
		m->floating = false;
		m->offset = 0;
		return;
	}

	m->floating = true;

	int minOffset = -(fm.width(m->text) - this->width() + tolerance);
	int maxOffset = tolerance;

	int charWidth = fm.width("O");
	int charsWidth = m->chars_per_second * charWidth;

	if(difference < charsWidth)
	{
		charsWidth = difference / 2;
	}

	double pixelsPerMSecond = (charsWidth) / 1000.0;
	double pixelsPerIntervalStep = pixelsPerMSecond * timerInterval;

	m->offset += ( m->offset_direction * pixelsPerIntervalStep );
	if(m->offset < minOffset || m->offset > maxOffset)
	{
		m->offset_direction = -m->offset_direction;

		if(m->offset < minOffset)
		{
			m->offset = minOffset;
		}

		else {
			m->offset = maxOffset;
		}
	}

	this->update();
}
