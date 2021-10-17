/* GUI_AudioConverter.cpp */

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

#include "GUI_AudioConverter.h"
#include "Gui/Plugins/ui_GUI_AudioConverter.h"

#include "Utils/Utils.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/DirectoryChooser.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Converter/ConverterFactory.h"
#include "Components/Converter/Converter.h"

#include <QStringList>

namespace
{
	enum StackedWidgetPage
	{
		Ogg = 0,
		OpusCBR,
		OpusVBR,
		LameCBR,
		LameVBR
	};

	Converter* createConverter(Ui::GUI_AudioConverter* ui, ConverterFactory* converterFactory)
	{
		switch(ui->sw_preferences->currentIndex())
		{
			case StackedWidgetPage::Ogg:
				return converterFactory->createConverter<ConverterFactory::ConvertType::OggVorbis>(
				                                         ui->sb_ogg_quality->value());
			case StackedWidgetPage::LameCBR:
				return converterFactory->createConverter<ConverterFactory::ConvertType::Lame>(
				                                         ConverterFactory::Bitrate::Constant,
				                                         ui->combo_cbr->currentText().toInt());
			case StackedWidgetPage::LameVBR:
				return converterFactory->createConverter<ConverterFactory::ConvertType::Lame>(
				                                         ConverterFactory::Bitrate::Variable,
				                                         ui->sb_lame_vbr->value());
			case StackedWidgetPage::OpusCBR:
				return converterFactory->createConverter<ConverterFactory::ConvertType::OggOpus>(
				                                         ConverterFactory::Bitrate::Constant,
				                                         ui->combo_opus_cbr->currentText().toInt());
			case StackedWidgetPage::OpusVBR:
				return converterFactory->createConverter<ConverterFactory::ConvertType::OggOpus>(
				                                         ConverterFactory::Bitrate::Variable,
				                                         ui->combo_opus_vbr->currentText().toInt());
		}

		return nullptr;
	}
}

struct GUI_AudioConverter::Private
{
	ConverterFactory* converterFactory;
	bool isLameAvailable;

	Private(ConverterFactory* converterFactory) :
		converterFactory(converterFactory),
		isLameAvailable(true) {}
};

GUI_AudioConverter::GUI_AudioConverter(ConverterFactory* converterFactory, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(converterFactory);
}

GUI_AudioConverter::~GUI_AudioConverter()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_AudioConverter::initUi()
{
	setupParent(this, &ui);

	const QString preferredConverter = GetSetting(Set::AudioConvert_PreferredConverter);
	int idx = std::max(ui->combo_codecs->findText(preferredConverter), 0);

	ui->combo_codecs->setCurrentIndex(idx);
	ui->sw_preferences->setCurrentIndex(idx);

	ui->sw_progress->setCurrentIndex(0);
	ui->sb_threads->setValue(GetSetting(Set::AudioConvert_NumberThreads));

	int lame_cbr = GetSetting(Set::AudioConvert_QualityLameCBR);
	ui->combo_cbr->setCurrentIndex(ui->combo_cbr->findText(QString::number(lame_cbr)));

	ui->sb_lame_vbr->setValue(GetSetting(Set::AudioConvert_QualityLameVBR));
	ui->sb_ogg_quality->setValue(GetSetting(Set::AudioConvert_QualityOgg));

	connect(ui->btn_start, &QPushButton::clicked, this, &GUI_AudioConverter::btnStartClicked);
	connect(ui->combo_codecs, combo_activated_int, this, &GUI_AudioConverter::comboCodecsIndexChanged);
	connect(ui->combo_cbr, combo_activated_int, this, &GUI_AudioConverter::comboLameCbrIndexChanged);

	connect(ui->sb_ogg_quality, spinbox_value_changed_int, this, &GUI_AudioConverter::oggQualityChanged);
	connect(ui->sb_lame_vbr, spinbox_value_changed_int, this, &GUI_AudioConverter::comboLameVbrIndexChanged);
	connect(ui->sb_threads, spinbox_value_changed_int, this, &GUI_AudioConverter::threadCountChanged);

	checkStartButton();
}

QString GUI_AudioConverter::name() const
{
	return "Audio Converter";
}

QString GUI_AudioConverter::displayName() const
{
	return tr("Audio Converter");
}

void GUI_AudioConverter::retranslate()
{
	int idxCodec = ui->combo_codecs->currentIndex();
	int idxCBR = ui->combo_cbr->currentIndex();

	ui->retranslateUi(this);
	ui->lab_threads->setText("#" + tr("Threads"));

	ui->combo_codecs->setCurrentIndex(idxCodec);
	ui->combo_cbr->setCurrentIndex(idxCBR);

	checkStartButton();
}

void GUI_AudioConverter::btnStartClicked()
{
	if(!isUiInitialized())
	{
		return;
	}

	int threadCount = GetSetting(Set::AudioConvert_NumberThreads);

	auto* converter = createConverter(ui, m->converterFactory);
	{ // create and check converter
		if(!converter)
		{
			return;
		}

		if(!converter->isAvailable())
		{
			Message::error(tr("Cannot find encoder") + (" '" + converter->binary() + "'"));
			converter->deleteLater();
			return;
		}
	}

	{ // check input files
		if(converter->fileCount() == 0)
		{
			Message::error
				(
					tr("Playlist does not contain tracks which are supported by the converter"),
					QString(" (%1). ").arg(converter->supportedInputFormats().join(", ")) +
					tr("No track will be converted.")
				);

			converter->deleteLater();
			return;
		}

		if(converter->initialCount() > converter->fileCount())
		{
			Message::warning
				(
					tr("Playlist does not contain tracks which are supported by the converter"),
					QString(" (%1). ").arg(converter->supportedInputFormats().join(", ")) +
					tr("These tracks will be ignored")
				);
		}
	}

	const auto convertTargetPath = GetSetting(Set::Engine_CovertTargetPath);
	const auto dir = Gui::DirectoryChooser::getDirectory(tr("Choose target directory"), convertTargetPath, true, this);
	if(dir.isEmpty())
	{
		converter->deleteLater();
		return;
	}

	SetSetting(Set::Engine_CovertTargetPath, dir);

	connect(converter, &Converter::sigFinished, this, &GUI_AudioConverter::convertFinished);
	connect(converter, &Converter::sigProgress, ui->pb_progress, &QProgressBar::setValue);
	connect(ui->btn_stop_encoding, &QPushButton::clicked, converter, &Converter::stop);
	connect(ui->btn_stop_encoding, &QPushButton::clicked, this, &GUI_AudioConverter::resetButtons);

	ui->pb_progress->setValue(0);
	ui->sw_progress->setCurrentIndex(1);

	converter->start(threadCount, dir);
}

void GUI_AudioConverter::convertFinished()
{
	auto* converter = static_cast<Converter*>(sender());
	if(converter)
	{
		const auto errorCount = converter->errorCount();
		if(errorCount > 0)
		{
			Message::error(QStringList
				               {
					               tr("Failed to convert %n track(s)", "", errorCount),
					               tr("Please check the log files") + ".",
					               Util::createLink(converter->loggingDirectory(), Style::isDark(), true, "file://" +
					                                                                                      converter->loggingDirectory())
				               }.join("<br>"));
		}

		else
		{
			Message::info(QStringList
				              {
					              tr("All tracks could be converted"),
					              QString(),
					              Util::createLink(converter->targetDirectory(), Style::isDark(), true)
				              }.join("<br>"));
		}

		converter->deleteLater();
	}

	else
	{
		Message::info(tr("Successfully finished"));
	}

	resetButtons();
}

void GUI_AudioConverter::comboCodecsIndexChanged(int idx)
{
	if(idx < 0)
	{
		return;
	}

	ui->sw_preferences->setCurrentIndex(idx);

	QString text = ui->combo_codecs->currentText();
	SetSetting(Set::AudioConvert_PreferredConverter, text);

	checkStartButton();
}

void GUI_AudioConverter::resetButtons()
{
	ui->sw_progress->setCurrentIndex(0);
}

void GUI_AudioConverter::threadCountChanged(int value)
{
	SetSetting(Set::AudioConvert_NumberThreads, value);
}

void GUI_AudioConverter::checkStartButton()
{
	auto* converter = createConverter(ui, m->converterFactory);
	if(!converter->isAvailable())
	{
		ui->btn_start->setEnabled(false);
		ui->btn_start->setText(tr("Cannot find encoder") + (" '" + converter->binary() + "'"));
	}

	else
	{
		ui->btn_start->setEnabled(true);
		ui->btn_start->setText(tr("Start"));
	}

	converter->deleteLater();
}

void GUI_AudioConverter::oggQualityChanged(int value)
{
	SetSetting(Set::AudioConvert_QualityOgg, value);
}

void GUI_AudioConverter::comboLameVbrIndexChanged(int value)
{
	SetSetting(Set::AudioConvert_QualityLameVBR, value);
}

void GUI_AudioConverter::comboLameCbrIndexChanged(int idx)
{
	Q_UNUSED(idx)
	SetSetting(Set::AudioConvert_QualityLameCBR, ui->combo_cbr->currentText().toInt());
}

