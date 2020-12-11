/* Equalizer.h */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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
#ifndef SAYONARA_PLAYER_EQUALIZER_H
#define SAYONARA_PLAYER_EQUALIZER_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QList>

class EqualizerSetting;
class QString;
class Equalizer : public QObject
{
	Q_OBJECT
	PIMPL(Equalizer)

	signals:
		void sigValueChanged(int band, int value);

	public:
		enum RenameError
		{
			NoError=0,
			DbError,
			EmptyName,
			NameAlreadyKnown,
			InvalidIndex
		};

		Equalizer(QObject* parent=nullptr);
		~Equalizer() noexcept;

		const EqualizerSetting& equalizerSetting(int index) const;
		EqualizerSetting& equalizerSetting(int index);
		const EqualizerSetting& currentSetting() const;

		QStringList names() const;
		QStringList defaultNames() const;

		void changeValue(int index, int band, int value);
		void resetPreset(int presetIndex);
		RenameError renamePreset(int index, const QString& newName);
		bool deletePreset(int presetIndex);
		RenameError saveCurrentEqualizerAs(const QString& name);

		int currentIndex() const;
		void setCurrentIndex(int index);

		int count() const;

		void setGaussEnabled(bool enabled);
		bool isGaussEnabled() const;

		void startValueChange();
		void endValueChange();
};

#endif //SAYONARA_PLAYER_EQUALIZER_H
