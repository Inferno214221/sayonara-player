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
	void sig_spectrum(const QList<float>& spectrum, MilliSeconds percent);
	void sig_finished();
	void sig_started();

public:
	explicit AudioDataProvider(QObject *parent = nullptr);
	~AudioDataProvider() override;

	void set_spectrum(const QList<float>& spectrum, NanoSeconds clock_time);
	GstElement* get_audioconvert() const;

	void start(const QString& filename);
	void stop();

	uint get_number_bins() const;
	void set_number_bins(uint num_bins);

	MilliSeconds get_interval_ms() const;
	void set_interval_ms(MilliSeconds ms);

	int get_threshold() const;
	void set_threshold(int threshold);

	void set_samplerate(uint samplerate);
	uint get_samplerate() const;

	float get_frequency(int bin);

	bool is_running() const;
	void set_running(bool b);

	bool is_finished(const QString& filename) const;
	void set_finished(bool b);
};

#endif // AUDIODATAPROVIDER_H
