/* GUI_ControlsBase.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
		virtual ~GUI_ControlsBase() override;
		virtual void init();

		virtual QLabel* labSayonara() const = 0;
		virtual QLabel* labTitle() const = 0;
		virtual QLabel* labVersion() const = 0;
		virtual QLabel* labAlbum() const = 0;
		virtual QLabel* labArtist() const = 0;
		virtual QLabel* labWrittenBy() const = 0;
		virtual QLabel* labBitrate() const = 0;
		virtual QLabel* labFilesize() const = 0;
		virtual QLabel* labCopyright() const = 0;
		virtual QLabel* labCurrentTime() const = 0;
		virtual QLabel* labMaxTime() const = 0;
		virtual Gui::RatingEditor* labRating() const;
		virtual QWidget* widgetDetails() const = 0;

		virtual Gui::SearchSlider* sliProgress() const = 0;
		virtual Gui::SearchSlider* sliVolume() const = 0;
		virtual QPushButton* btnMute() const = 0;
		virtual QPushButton* btnPlay() const = 0;
		virtual QPushButton* btnRecord() const = 0;
		virtual QPushButton* btnPrevious() const = 0;
		virtual QPushButton* btnNext() const = 0;
		virtual QPushButton* btnStop() const = 0;
		virtual Gui::CoverButton* btnCover() const = 0;

		virtual QSize buttonSize() const final;
		virtual bool isExternResizeAllowed() const = 0;

	private:
		QIcon icon(Gui::Icons::IconName name);

		void played();
		void paused();
		void stopped();

		void setCoverLocation(const MetaData& md);
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

	public slots:
		void changeVolumeByDelta(int val);
		void setCoverData(const QByteArray& coverData, const QString& mimeType) override;
		bool isActive() const override;

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

		void refreshLabels(const MetaData& md);
		void refreshCurrentTrack();

		// cover changed by engine
		void coverClickRejected();

		void streamRecorderActiveChanged();

	protected:

		MD::Interpretation metadataInterpretation() const override;
		MetaDataList infoDialogData() const override;

		void resizeEvent(QResizeEvent* e) override;
		void showEvent(QShowEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;
		void skinChanged() override;
};

#endif // GUI_CONTROLSBASE_H
