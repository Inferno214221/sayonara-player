#include "GUI_SpectrogramPainter.h"

#include "Components/Engine/AudioDataProvider.h"
#include "Components/PlayManager/PlayManager.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"
#include "Gui/Utils/EventFilter.h"

#include <QPainter>
#include <QList>
#include <QDir>
#include <QLabel>

#include <cmath>
#include <algorithm>

using Dot=float; // brightness
using Line=QList<Dot>;

struct GUI_SpectrogramPainter::Private
{
	QPixmap pm;

	Line current_line;

	int promille_values;
	int current_promille;

	AudioDataProvider* adp=nullptr;
	QString filename;

	Private() :
		promille_values(0),
		current_promille(-1)
	{
		pm = QPixmap(800, 800);
	}

	void scale_current_line()
	{
		for(auto it=current_line.begin(); it != current_line.end(); it++)
		{
			*it /= promille_values;
		}

		promille_values = 1;
	}
};

GUI_SpectrogramPainter::GUI_SpectrogramPainter(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
	m->adp = new AudioDataProvider(this);

	this->setMouseTracking(true);

	connect(m->adp, &AudioDataProvider::sig_started, this, &GUI_SpectrogramPainter::reset);
	connect(m->adp, &AudioDataProvider::sig_spectrum, this, &GUI_SpectrogramPainter::spectrum_changed);
	connect(m->adp, &AudioDataProvider::sig_finished, this, &GUI_SpectrogramPainter::finished);

	auto* pm = PlayManager::instance();
	connect(pm, &PlayManager::sig_track_changed, this, &GUI_SpectrogramPainter::track_changed);
	connect(pm, &PlayManager::sig_playstate_changed, this, &GUI_SpectrogramPainter::playstate_changed);
}

GUI_SpectrogramPainter::~GUI_SpectrogramPainter() = default;

QString GUI_SpectrogramPainter::get_name() const
{
	return "spectrogram_painter";
}

QString GUI_SpectrogramPainter::get_display_name() const
{
	return tr("Spectrogram");
}

bool GUI_SpectrogramPainter::is_ui_initialized() const
{
	return true;
}

void GUI_SpectrogramPainter::spectrum_changed(const QList<float>& spectrum, MilliSeconds ms)
{
	double promille = (ms * 1000.0) / PlayManager::instance()->current_track().duration_ms();

	if(m->current_promille == int(promille))
	{
		std::transform(	m->current_line.begin(), m->current_line.end(),
						spectrum.begin(), m->current_line.begin(), std::plus<float>());

		m->promille_values++;
	}

	else
	{
		if(m->current_promille >= 0) {
			draw_buffer(int(promille) - m->current_promille);
		}

		m->current_line = spectrum;
		m->promille_values = 1;
		m->current_promille = int(promille);
	}
}


void GUI_SpectrogramPainter::draw_buffer(int percent_step)
{
	QPainter p(&m->pm);
	m->scale_current_line();

	const int Threshold = m->adp->get_threshold();
	const int stretch_factor = int((m->pm.height() * 1.0) / m->adp->get_number_bins());
	const int x = int((m->pm.width() * m->current_promille) / 1000.0);

	static const double brightest = (256 + 256 + 512);

	int cur_offset = m->pm.height();
	for(Dot dot : m->current_line)
	{
		if(dot >= 0) {
			continue;
		}

		dot = std::max(-74.99f, dot);

		float delta = (dot / (Threshold * 1.0f));	// [0; 1]
		float l = 1.0f - delta;

		int color_value = int(std::min<float>(brightest, (brightest * l)));
		int r = std::min(color_value, 255);						// if color_value > 0 paint some red
		int g = std::max(0, std::min(color_value - 256, 255));  // if color_value > 255 paint some green. Because r is full -> yellowish
		int b = std::max(0, std::min((color_value - 512), 255)); // if color_value > 512 paint some blue, too. Because r and g is full -> whitish
		int a = 255;
		if(color_value < 128){
			a = color_value;
		}

		QColor col(r, g, b, a);
		p.setBrush(col);
		p.setPen(col);

		for(int i=0; i<percent_step; i++)
		{
			p.drawLine(x+i, cur_offset, x+i, cur_offset - stretch_factor);
		}

		cur_offset -= stretch_factor;
	}

	p.end();

	this->repaint();
}

QString GUI_SpectrogramPainter::calc_tooltip(float yPercent)
{
	int bin = int(m->current_line.count() * yPercent);

	float freq1 = -1;
	if(bin > 0){
		freq1 = m->adp->get_frequency(bin-1);
	};
	float freq2 = m->adp->get_frequency(bin);

	return QString("%1 - %2 Hz").arg(freq1+1.0f).arg(freq2);
}

QSize GUI_SpectrogramPainter::minimumSizeHint() const
{
	return QSize(200, 100);
}

void GUI_SpectrogramPainter::finished() {}

void GUI_SpectrogramPainter::playstate_changed(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->adp->stop();
	}
}

void GUI_SpectrogramPainter::track_changed(const MetaData& md)
{
	if(this->isVisible())
	{
		start_adp(md);
	}
}

void GUI_SpectrogramPainter::reset()
{
	m->pm.fill(QColor(0, 0, 0, 0));
	m->current_promille = -1;
}


void GUI_SpectrogramPainter::show_fullsize()
{
	QLabel* label = new QLabel(nullptr);
	QPoint p = QCursor::pos();

	label->setPixmap(m->pm);
	label->setGeometry(p.x(), p.y(), m->pm.width(), m->pm.height());
	label->setScaledContents(true);
	label->setMouseTracking(true);
	label->show();
	label->setToolTipDuration(0);

	auto* gf = new Gui::GenericFilter(QEvent::Close, label);
	auto* mmf = new Gui::MouseMoveFilter(label);
	label->installEventFilter(gf);
	label->installEventFilter(mmf);

	connect(gf, &Gui::GenericFilter::sig_event, this, [=](auto unused){
		Q_UNUSED(unused);
		label->deleteLater();
	});

	connect(mmf, &Gui::MouseMoveFilter::sig_mouse_moved, this, [=](const QMouseEvent* e)
	{
		QPoint p = e->pos();
		float yPercent = (1.0f - (p.y() * 1.0f) / label->height());
		label->setToolTip( calc_tooltip(yPercent) );
	});
}

void GUI_SpectrogramPainter::position_clicked(QPoint position)
{
	double percent = (position.x() * 1.0) / this->width();
	PlayManager::instance()->seek_rel(percent);
}

void GUI_SpectrogramPainter::start_adp(const MetaData& md)
{
	if(m->adp->is_finished(md.filepath())){
		return;
	}

	stop_adp();

	m->filename = md.filepath();
	m->adp->start(md.filepath());
}

void GUI_SpectrogramPainter::stop_adp()
{
	m->adp->stop();
}

void GUI_SpectrogramPainter::showEvent(QShowEvent* e)
{
	PlayerPlugin::Base::showEvent(e);

	if(PlayManager::instance()->playstate() != PlayState::Stopped)
	{
		start_adp(PlayManager::instance()->current_track());
	}
}

void GUI_SpectrogramPainter::closeEvent(QCloseEvent* e)
{
	stop_adp();
	PlayerPlugin::Base::closeEvent(e);
}

void GUI_SpectrogramPainter::paintEvent(QPaintEvent* e)
{
	e->accept();

	QPainter p(this);
	p.drawPixmap(0, 0, this->width(), this->height(), m->pm);
}

void GUI_SpectrogramPainter::mousePressEvent(QMouseEvent* e)
{
	if(e->button() & Qt::LeftButton)
	{
		position_clicked(e->pos());
	}

	else if(e->button() & Qt::MiddleButton)
	{
		show_fullsize();
	}

	PlayerPlugin::Base::mousePressEvent(e);
}

void GUI_SpectrogramPainter::mouseMoveEvent(QMouseEvent* e)
{
	float yPercent = (1.0f - (e->pos().y() * 1.0f) / this->height());
	this->setToolTip( calc_tooltip(yPercent) );

	PlayerPlugin::Base::mouseMoveEvent(e);
}

void GUI_SpectrogramPainter::retranslate_ui() {}

void GUI_SpectrogramPainter::init_ui() {}


