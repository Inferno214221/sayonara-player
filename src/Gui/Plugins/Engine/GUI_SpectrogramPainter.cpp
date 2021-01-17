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
	PlayManager* playManager = nullptr;
	AudioDataProvider* audioDataProvider=nullptr;
	QString filename;
	QPixmap pm;
	Line currentLine;

	int promilleValues;
	int currentPromille;

	Private() :
		playManager(PlayManagerProvider::instance()->playManager()),
		promilleValues(0),
		currentPromille(-1)
	{
		pm = QPixmap(800, 800);
	}

	void scaleCurrentLine()
	{
		for(auto it=currentLine.begin(); it != currentLine.end(); it++)
		{
			*it /= promilleValues;
		}

		promilleValues = 1;
	}
};

GUI_SpectrogramPainter::GUI_SpectrogramPainter(QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>();
	m->audioDataProvider = new AudioDataProvider(this);

	this->setMouseTracking(true);

	connect(m->audioDataProvider, &AudioDataProvider::sigStarted, this, &GUI_SpectrogramPainter::reset);
	connect(m->audioDataProvider, &AudioDataProvider::sigSpectrumDataAvailable, this, &GUI_SpectrogramPainter::spectrumChanged);
	connect(m->audioDataProvider, &AudioDataProvider::sigFinished, this, &GUI_SpectrogramPainter::finished);

	auto* playManager = PlayManagerProvider::instance()->playManager();
	connect(playManager, &PlayManager::sigCurrentTrackChanged, this, &GUI_SpectrogramPainter::trackChanged);
	connect(playManager, &PlayManager::sigPlaystateChanged, this, &GUI_SpectrogramPainter::playstateChanged);
}

GUI_SpectrogramPainter::~GUI_SpectrogramPainter() = default;

QString GUI_SpectrogramPainter::name() const
{
	return "spectrogram_painter";
}

QString GUI_SpectrogramPainter::displayName() const
{
	return tr("Spectrogram");
}

bool GUI_SpectrogramPainter::isUiInitialized() const
{
	return true;
}

void GUI_SpectrogramPainter::spectrumChanged(const QList<float>& spectrum, MilliSeconds ms)
{
	double promille = (ms * 1000.0) / m->playManager->currentTrack().durationMs();

	if(m->currentPromille == int(promille))
	{
		std::transform(	m->currentLine.begin(), m->currentLine.end(),
						spectrum.begin(), m->currentLine.begin(), std::plus<float>());

		m->promilleValues++;
	}

	else
	{
		if(m->currentPromille >= 0) {
			drawBuffer(int(promille) - m->currentPromille);
		}

		m->currentLine = spectrum;
		m->promilleValues = 1;
		m->currentPromille = int(promille);
	}
}


void GUI_SpectrogramPainter::drawBuffer(int percent_step)
{
	QPainter p(&m->pm);
	m->scaleCurrentLine();

	const int Threshold = m->audioDataProvider->threshold();
	const int stretch_factor = int((m->pm.height() * 1.0) / m->audioDataProvider->binCount());
	const int x = int((m->pm.width() * m->currentPromille) / 1000.0);

	static const double brightest = (256 + 256 + 512);

	int cur_offset = m->pm.height();
	for(Dot dot : m->currentLine)
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

QString GUI_SpectrogramPainter::calcTooltip(float yPercent)
{
	int bin = int(m->currentLine.count() * yPercent);

	float freq1 = -1;
	if(bin > 0){
		freq1 = m->audioDataProvider->frequency(bin-1);
	};
	float freq2 = m->audioDataProvider->frequency(bin);

	return QString("%1 - %2 Hz").arg(freq1+1.0f).arg(freq2);
}

QSize GUI_SpectrogramPainter::minimumSizeHint() const
{
	return QSize(200, 100);
}

void GUI_SpectrogramPainter::finished() {}

void GUI_SpectrogramPainter::playstateChanged(PlayState state)
{
	if(state == PlayState::Stopped)
	{
		m->audioDataProvider->stop();
	}
}

void GUI_SpectrogramPainter::trackChanged(const MetaData& md)
{
	if(this->isVisible())
	{
		startAudioDataProvider(md);
	}
}

void GUI_SpectrogramPainter::reset()
{
	m->pm.fill(QColor(0, 0, 0, 0));
	m->currentPromille = -1;
}


void GUI_SpectrogramPainter::showFullsize()
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

	connect(gf, &Gui::GenericFilter::sigEvent, this, [=](auto unused){
		Q_UNUSED(unused);
		label->deleteLater();
	});

	connect(mmf, &Gui::MouseMoveFilter::sigMouseMoved, this, [=](const QMouseEvent* e)
	{
		QPoint p = e->pos();
		float yPercent = (1.0f - (p.y() * 1.0f) / label->height());
		label->setToolTip( calcTooltip(yPercent) );
	});
}

void GUI_SpectrogramPainter::positionClicked(QPoint position)
{
	double percent = (position.x() * 1.0) / this->width();
	m->playManager->seekRelative(percent);
}

void GUI_SpectrogramPainter::startAudioDataProvider(const MetaData& md)
{
	if(m->audioDataProvider->isFinished(md.filepath())){
		return;
	}

	stopAudioDataProvider();

	m->filename = md.filepath();
	m->audioDataProvider->start(md.filepath());
}

void GUI_SpectrogramPainter::stopAudioDataProvider()
{
	m->audioDataProvider->stop();
}

void GUI_SpectrogramPainter::showEvent(QShowEvent* e)
{
	PlayerPlugin::Base::showEvent(e);

	if(m->playManager->playstate() != PlayState::Stopped)
	{
		startAudioDataProvider(m->playManager->currentTrack());
	}
}

void GUI_SpectrogramPainter::closeEvent(QCloseEvent* e)
{
	stopAudioDataProvider();
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
		positionClicked(e->pos());
	}

	else if(e->button() & Qt::MiddleButton)
	{
		showFullsize();
	}

	PlayerPlugin::Base::mousePressEvent(e);
}

void GUI_SpectrogramPainter::mouseMoveEvent(QMouseEvent* e)
{
	float yPercent = (1.0f - (e->pos().y() * 1.0f) / this->height());
	this->setToolTip( calcTooltip(yPercent) );

	PlayerPlugin::Base::mouseMoveEvent(e);
}

void GUI_SpectrogramPainter::retranslate() {}

void GUI_SpectrogramPainter::initUi() {}


