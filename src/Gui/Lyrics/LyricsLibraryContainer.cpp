/* LyricsLibraryContainer.cpp, (Created on 04.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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
#include "LyricsLibraryContainer.h"

#include "Components/PlayManager/PlayManager.h"
#include "Gui/Lyrics/GUI_Lyrics.h"
#include "Gui/Utils/Icons.h"
#include "Utils/Language/Language.h"

#include <QFrame>
#include <QIcon>
#include <QLayout>
#include <QWidget>

namespace
{
	class LyricsWidget :
		public QWidget
	{
		Q_OBJECT

		public:
			explicit LyricsWidget(PlayManager* playManager) :
				m_playManager {playManager}
			{
				auto* mainLayout = new QVBoxLayout();
				auto* headerLayout = new QHBoxLayout();

				mainLayout->addLayout(headerLayout);
				mainLayout->setSpacing(5);
				mainLayout->addWidget(m_lyrics);
				setLayout(mainLayout);

				headerLayout->addWidget(m_frame);
				headerLayout->addItem(new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding));

				connect(m_playManager, &PlayManager::sigCurrentTrackChanged, m_lyrics, [this](const auto& track) {
					if(isVisible())
					{
						m_lyrics->setTrack(track);
					}
				});

				connect(m_playManager, &PlayManager::sigCurrentMetadataChanged, m_lyrics, [this]() {
					if(isVisible())
					{
						m_lyrics->setTrack(m_playManager->currentTrack());
					}
				});

				m_lyrics->setTrack(playManager->currentTrack());
			}

			~LyricsWidget() override = default;

			QFrame* frame() { return m_frame; }

			GUI_Lyrics* lyrics() { return m_lyrics; }

		protected:
			void showEvent(QShowEvent* event) override
			{
				m_lyrics->setTrack(m_playManager->currentTrack());
				QWidget::showEvent(event);
			}

		private:
			QFrame* m_frame {new QFrame(this)};
			GUI_Lyrics* m_lyrics {new GUI_Lyrics(false, nullptr)};
			PlayManager* m_playManager;
	};
}

struct LyricsLibraryContainer::Private
{
	LyricsWidget* lyricsWidget;

	explicit Private(PlayManager* playManager) :
		lyricsWidget {new LyricsWidget(playManager)} {}
};

LyricsLibraryContainer::LyricsLibraryContainer(PlayManager* playManager, Library::PluginHandler* libraryPluginHandler) :
	Gui::Library::Container {libraryPluginHandler},
	m {Pimpl::make<Private>(playManager)} {}

QFrame* LyricsLibraryContainer::header() const { return m->lyricsWidget->frame(); }

QIcon LyricsLibraryContainer::icon() const { return Gui::Icons::icon(Gui::Icons::Lyrics); }

QMenu* LyricsLibraryContainer::menu() { return nullptr; }

QString LyricsLibraryContainer::displayName() const { return Lang::get(Lang::Lyrics); }

QString LyricsLibraryContainer::name() const { return "lyrics"; }

QWidget* LyricsLibraryContainer::widget() const { return m->lyricsWidget; }

bool LyricsLibraryContainer::isLocal() const { return false; }

void LyricsLibraryContainer::rename(const QString& /*newName*/) {}

void LyricsLibraryContainer::initUi() {}

#include "LyricsLibraryContainer.moc"
