/* GUI_ControlsBase.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef GUI_CONTROLSBASE_H
#define GUI_CONTROLSBASE_H

#include "Interfaces/Engine/CoverDataReceiver.h"

#include "Gui/InfoDialog/InfoDialogContainer.h"
#include "Gui/Utils/Widgets/Widget.h"
#include "Gui/Utils/Icons.h"

#include "Utils/MetaData/RadioMode.h"
#include "Utils/Pimpl.h"

class QLabel;
class QSlider;
class QPushButton;

class CoverDataProvider;
class PlayManager;

namespace Gui
{
	class CoverButton;
	class SearchSlider;
	class ProgressBar;
	class RatingEditor;
}

class GUI_ControlsBase :
	public Gui::Widget,
	public InfoDialogContainer,
	public CoverDataReceiver
{
	Q_OBJECT
	PIMPL(GUI_ControlsBase)

	public:
		GUI_ControlsBase(PlayManager* playManager, CoverDataProvider* coverProvider, QWidget* parent = nullptr);
		~GUI_ControlsBase() override;
		virtual void init();

		[[nodiscard]] virtual QLabel* labSayonara() const = 0;
		[[nodiscard]] virtual QLabel* labTitle() const = 0;
		[[nodiscard]] virtual QLabel* labVersion() const = 0;
		[[nodiscard]] virtual QLabel* labAlbum() const = 0;
		[[nodiscard]] virtual QLabel* labArtist() const = 0;
		[[nodiscard]] virtual QLabel* labWrittenBy() const = 0;
		[[nodiscard]] virtual QLabel* labBitrate() const = 0;
		[[nodiscard]] virtual QLabel* labFilesize() const = 0;
		[[nodiscard]] virtual QLabel* labCopyright() const = 0;
		[[nodiscard]] virtual QLabel* labCurrentTime() const = 0;
		[[nodiscard]] virtual QLabel* labMaxTime() const = 0;
		[[nodiscard]] virtual Gui::RatingEditor* labRating() const;
		[[nodiscard]] virtual QWidget* widgetDetails() const = 0;

		[[nodiscard]] virtual Gui::SearchSlider* sliProgress() const = 0;
		[[nodiscard]] virtual Gui::SearchSlider* sliVolume() const = 0;
		[[nodiscard]] virtual QPushButton* btnMute() const = 0;
		[[nodiscard]] virtual QPushButton* btnPlay() const = 0;
		[[nodiscard]] virtual QPushButton* btnRecord() const = 0;
		[[nodiscard]] virtual QPushButton* btnPrevious() const = 0;
		[[nodiscard]] virtual QPushButton* btnNext() const = 0;
		[[nodiscard]] virtual QPushButton* btnStop() const = 0;
		[[nodiscard]] virtual Gui::CoverButton* btnCover() const = 0;

		[[nodiscard]] virtual bool isExternResizeAllowed() const = 0;

	private:
		void played();
		void paused();
		void stopped();

		void setCoverLocation(const MetaData& track);
		void setStandardCover();

		void setRadioMode(RadioMode radio);
		void checkRecordButtonVisible();

		void setupVolumeButton(int percent);
		void increaseVolume();
		void decreaseVolume();

		void refreshCurrentPosition(int val);
		void setTotalTimeLabel(MilliSeconds totalTimeMs);

		void setupShortcuts();
		void setupConnections();

		void showTrackInfo(bool b);

	public slots:
		void changeVolumeByDelta(int val);
		void setCoverData(const QByteArray& coverData, const QString& mimeType) override;

	private slots:
		void playstateChanged(PlayState state);

		void recordChanged(bool b);

		void buffering(int progress);

		void currentPositionChanged(MilliSeconds posMs);
		void progressMoved(int val);
		void progressHovered(int val);

		void volumeChanged(int val);
		void muteChanged(bool muted);

		void currentTrackChanged(const MetaData& track);
		void metadataChanged();

		void refreshLabels(const MetaData& track);
		void refreshCurrentTrack();

		// cover changed by engine
		void coverClickRejected();

		void streamRecorderActiveChanged();

	protected:
		[[nodiscard]] bool isActive() const override;
		[[nodiscard]] MD::Interpretation metadataInterpretation() const override;
		[[nodiscard]] MetaDataList infoDialogData() const override;
		QWidget* getParentWidget() override;

		void resizeEvent(QResizeEvent* e) override;
		void showEvent(QShowEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;
		void skinChanged() override;
};

#endif // GUI_CONTROLSBASE_H
