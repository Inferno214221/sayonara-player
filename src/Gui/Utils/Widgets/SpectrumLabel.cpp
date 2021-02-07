#include "SpectrumLabel.h"
#include "Components/Engine/EngineHandler.h"
#include <cmath>

#include <QPixmap>
#include <QPainter>
#include <QEvent>

struct SpectrumLabel::Private
{
	Engine::Handler* engine;

	Private(Engine::Handler* engine) :
		engine(engine) {}
};

SpectrumLabel::SpectrumLabel(Engine::Handler* engine, QWidget* parent) :
	QLabel(parent)
{
	m = Pimpl::make<Private>(engine);

	m->engine->registerSpectrumReceiver(this);
}

SpectrumLabel::~SpectrumLabel() = default;

void SpectrumLabel::setSpectrum(const std::vector<float>& spectrum)
{
	QPixmap pm(this->width(), this->height());
	pm.fill(QColor(0, 0, 0, 0));

	QPainter painter(&pm);

	double bass = (spectrum[1] + 75.0) / 75.0;
	double midTmp = (spectrum[spectrum.size() / 2] + 75.0) / 75.0;
	double highTmp = (spectrum[spectrum.size() - 2] + 76.0) / 75.0;

	double mid = std::pow(midTmp, 0.5);
	double high = std::pow(highTmp, 0.3) * 2.0;

	int w = this->width() / 3 - 3;
	int h = this->height();
	// left, top, width, height
	QRect bassRect(2, h - int(h * bass), w, h);
	QRect midRect(2 + w, h - int(h * mid), w, h);
	QRect highRect(2 + w + w, h - int(h * high), w, h);

	painter.setBrush(QColor(255, 255, 255));
	painter.drawRect(bassRect);
	painter.drawRect(midRect);
	painter.drawRect(highRect);

	this->setPixmap(pm);

	emit sigPixmapChanged();
}

bool SpectrumLabel::isActive() const
{
	return this->isVisible();
}
