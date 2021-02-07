#ifndef SPECTRUMLABEL_H
#define SPECTRUMLABEL_H

#include "Interfaces/Engine/AudioDataReceiverInterface.h"

#include "Utils/Pimpl.h"

#include <QLabel>
#include <vector>

namespace Engine
{
	class Handler;
}

class SpectrumLabel :
	public QLabel,
	public Engine::SpectrumReceiver
{
	Q_OBJECT
	PIMPL(SpectrumLabel)

	signals:
		void sigPixmapChanged();

	public:
		SpectrumLabel(Engine::Handler* engine, QWidget* parent);
		~SpectrumLabel() override;

		void setSpectrum(const std::vector<float>& spectrum) override;
		bool isActive() const override;
};

#endif // SPECTRUMLABEL_H
