/* AudioDataProvider.h
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

#ifndef AUDIODATAPROVIDER_H
#define AUDIODATAPROVIDER_H

#include <QObject>
#include <QList>

#include "Utils/Pimpl.h"
#include <gst/gst.h>

class AudioDataProvider :
	public QObject
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
		void setBinCount(uint numBins);

		MilliSeconds intervalMs() const;
		void setIntervalMs(MilliSeconds ms);

		int threshold() const;
		void setThreshold(int threshold);

		void setSamplerate(uint samplerate);
		uint samplerate() const;

		float frequency(int bins);

		bool isRunning() const;
		void setRunning(bool b);

		bool isFinished(const QString& filename) const;
		void setFinished(bool b);
};

#endif // AUDIODATAPROVIDER_H
