/* GUI_SpectrogramPainter.h
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

#ifndef GUI_SPECTROGRAMPAINTER_H
#define GUI_SPECTROGRAMPAINTER_H

#include "Gui/Plugins/PlayerPluginBase.h"

#include <QList>
#include <QWidget>

class PlayManager;

class GUI_SpectrogramPainter :
	public PlayerPlugin::Base
{
	Q_OBJECT
	PIMPL(GUI_SpectrogramPainter)

public:
	explicit GUI_SpectrogramPainter(PlayManager* playManager, QWidget* parent = nullptr);
	~GUI_SpectrogramPainter() override;

	QString name() const override;
	QString displayName() const override;
	bool isUiInitialized() const override;

private slots:
	void reset();
	void spectrumChanged(const QList<float>& spectrum, MilliSeconds ms);
	void finished();

	void playstateChanged(PlayState state);
	void trackChanged(const MetaData& md);

protected:
	void retranslate() override;
	void initUi() override;

	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;

	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

private:
	void drawBuffer(int percent_step);
	QString calcTooltip(float yPercent);

	void showFullsize();
	void positionClicked(QPoint position);

	void startAudioDataProvider(const MetaData& md);
	void stopAudioDataProvider();

	QSize minimumSizeHint() const override;
};

#endif // GUI_SPECTROGRAMPAINTER_H
