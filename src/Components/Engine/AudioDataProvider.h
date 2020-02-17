#ifndef AUDIODATAPROVIDER_H
#define AUDIODATAPROVIDER_H

#include <QObject>
#include <QList>

#include "Utils/Pimpl.h"
#include <gst/gst.h>

class AudioDataProvider : public QObject
{
	Q_OBJECT
	PIMPL(AudioDataProvider)

signals:
	void sigSpectrumDataAvailable(const QList<float>& spectrum, MilliSeconds percent);
	void sigFinished();
	void sigStarted();

public:
	explicit AudioDataProvider(QObject* parent = nullptr);
	~AudioDataProvider() override;

	void setSpectrum(const QList<float>& spectrum, NanoSeconds clock_time);
	GstElement* getAudioconverter() const;

	void start(const QString& filename);
	void stop();

	uint binCount() const;
	void setBinCount(uint num_bins);

	MilliSeconds intervalMs() const;
	void setIntervalMs(MilliSeconds ms);

	int threshold() const;
	void setThreshold(int threshold);

	void setSamplerate(uint samplerate);
	uint samplerate() const;

	float frequency(int bin);

	bool isRunning() const;
	void setRunning(bool b);

	bool isFinished(const QString& filename) const;
	void setFinished(bool b);
};

#endif // AUDIODATAPROVIDER_H
