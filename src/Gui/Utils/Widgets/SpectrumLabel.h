#ifndef SPECTRUMLABEL_H
#define SPECTRUMLABEL_H

#include <QLabel>
#include "Interfaces/Engine/AudioDataReceiverInterface.h"

class SpectrumLabel :
	public QLabel,
	public Engine::SpectrumReceiver
{
	Q_OBJECT

	signals:
		void sigPixmapChanged();

	public:
		SpectrumLabel(QWidget* parent);
		~SpectrumLabel() override;

		void setSpectrum(const Engine::SpectrumList& spectrum) override;
		bool isActive() const override;

	protected:
		bool event(QEvent* e) override;
};

#endif // SPECTRUMLABEL_H
