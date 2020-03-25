/* PlaybackEngine.cpp */

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

#include "Engine.h"
#include "Components/Engine/Callbacks.h"
#include "Components/Engine/Pipeline.h"
#include "Components/Engine/EngineUtils.h"

#include "StreamRecorder/StreamRecorder.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/FileUtils.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"

#include <QUrl>
#include <QList>

#include <algorithm>
#include <exception>

namespace EngineNS=Engine;
using EngineNS::Pipeline;
using EngineNS::PipelinePtr;
using EngineClass=EngineNS::Engine;
namespace EngineUtils=EngineNS::Utils;

struct PipelineCreationException : public std::exception {
   const char* what() const noexcept override;
};

const char* PipelineCreationException::what() const noexcept {
	return "Pipeline could not be created";
}

struct EngineClass::Private
{
	MetaData		md;
	PipelinePtr		pipeline, otherPipeline;

	SpectrumList		spectrumValues;
	QPair<float, float> levelValues;

	StreamRecorder::StreamRecorder*	streamRecorder=nullptr;

	MilliSeconds	currentPositionMs;
	GaplessState	gaplessState;

	Private() :
		currentPositionMs(0),
		gaplessState(GaplessState::Stopped)
	{}

	void changeGaplessState(GaplessState state)
	{
		Playlist::Mode plm = GetSetting(Set::PL_Mode);

		bool gapless = Playlist::Mode::isActiveAndEnabled(plm.gapless());
		bool crossfader = GetSetting(Set::Engine_CrossFaderActive);

		this->gaplessState = state;

		if(!gapless && !crossfader) {
			this->gaplessState = GaplessState::NoGapless;
		}
	}
};

EngineClass::Engine(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	gst_init(nullptr, nullptr);

	m->pipeline = initPipeline("FirstPipeline");
	if(!m->pipeline) {
		throw PipelineCreationException();
	}

	QString sink = GetSetting(Set::Engine_Sink);
	if(sink == "alsa")
	{
		Playlist::Mode plm = GetSetting(Set::PL_Mode);
		plm.setGapless(false, false);

		SetSetting(Set::Engine_CrossFaderActive, false);
		SetSetting(Set::PL_Mode, plm);
	}

	ListenSetting(Set::Engine_SR_Active, Engine::streamrecorderActiveChanged);
	ListenSetting(Set::PL_Mode, Engine::gaplessChanged);
	ListenSetting(Set::Engine_CrossFaderActive, Engine::gaplessChanged);
}


EngineClass::~Engine()
{
	if(isStreamRecorderRecording())
	{
		setStreamRecorderRecording(false);
	}

	if(m->streamRecorder)
	{
		m->streamRecorder->deleteLater();
		m->streamRecorder = nullptr;
	}
}

PipelinePtr EngineClass::initPipeline(const QString& name)
{
	PipelinePtr pipeline = std::make_shared<Pipeline>(name);
	if(!pipeline->init(this))
	{
		m->changeGaplessState(GaplessState::NoGapless);
		return nullptr;
	}

	connect(pipeline.get(), &Pipeline::sigAboutToFinishMs, this, &Engine::setTrackAlmostFinished);
	connect(pipeline.get(), &Pipeline::sigPositionChangedMs, this, &Engine::currentPositionChanged);
	connect(pipeline.get(), &Pipeline::sigDataAvailable, this, &Engine::sigDataAvailable);

	return pipeline;
}


void Engine::Engine::swapPipelines()
{
	m->otherPipeline->setVisualizerEnabled
	(
		m->pipeline->isLevelVisualizerEnabled(),
		m->pipeline->isSpectrumVisualizerEnabled()
	);

	m->otherPipeline->setBroadcastingEnabled
	(
		m->pipeline->isBroadcastingEnabled()
	);

	m->pipeline->setVisualizerEnabled(false, false);
	m->pipeline->setBroadcastingEnabled(false);

	std::swap(m->pipeline, m->otherPipeline);
}


bool EngineClass::changeTrackCrossfading(const MetaData& md)
{
	swapPipelines();

	m->otherPipeline->fadeOut();

	bool success = changeMetadata(md);
	if (success)
	{
		m->pipeline->fadeIn();
		m->changeGaplessState(GaplessState::Playing);
	}

	return success;
}

bool EngineClass::changeTrackGapless(const MetaData& md)
{
	swapPipelines();

	bool success = changeMetadata(md);
	if (success)
	{
		MilliSeconds timeToGo = m->otherPipeline->timeToGo();
		m->pipeline->playIn(timeToGo);

		m->changeGaplessState(GaplessState::TrackFetched);

		spLog(Log::Develop, this) << "Will start playing in " << timeToGo << "msec";
	}

	return success;
}

bool EngineClass::changeTrackImmediatly(const MetaData& md)
{
	if(m->otherPipeline) {
		m->otherPipeline->stop();
	}

	m->pipeline->stop();

	return changeMetadata(md);
}

bool EngineClass::changeTrack(const MetaData& md)
{
	if(!m->pipeline)
	{
		return false;
	}

	bool crossfader_active = GetSetting(Set::Engine_CrossFaderActive);
	if(m->gaplessState != GaplessState::Stopped && crossfader_active)
	{
		return changeTrackCrossfading(md);
	}

	else if(m->gaplessState == GaplessState::AboutToFinish)
	{
		return changeTrackGapless(md);
	}

	return changeTrackImmediatly(md);
}

bool EngineClass::changeMetadata(const MetaData& md)
{
	m->md = md;
	setCurrentPositionMs(0);

	const QString filepath = md.filepath();
	QString uri = filepath;

	bool playing_stream = Util::File::isWWW(filepath);
	if (playing_stream)
	{
		uri = QUrl(filepath).toString();
	}

	else if(!filepath.contains("://"))
	{
		QUrl url = QUrl::fromLocalFile(filepath);
		uri = url.toString();
	}

	if(uri.isEmpty())
	{
		m->md = MetaData();

		spLog(Log::Warning, this) << "uri = 0";
		return false;
	}

	bool success = m->pipeline->prepare(uri);
	if(!success)
	{
		m->changeGaplessState(GaplessState::Stopped);
	}

	return success;
}

void EngineClass::play()
{
	if( m->gaplessState == GaplessState::AboutToFinish ||
		m->gaplessState == GaplessState::TrackFetched)
	{
		return;
	}

	m->pipeline->play();

	if(isStreamRecorderRecording()) {
		setStreamRecorderRecording(true);
	}

	m->changeGaplessState(GaplessState::Playing);
}

void EngineClass::pause()
{
	m->pipeline->pause();
}

void EngineClass::stop()
{
	m->pipeline->stop();

	if(m->otherPipeline){
		m->otherPipeline->stop();
	}

	if(isStreamRecorderRecording()){
		setStreamRecorderRecording(false);
	}

	m->changeGaplessState(GaplessState::Stopped);
	m->currentPositionMs = 0;
	emit sigBuffering(-1);
}


void EngineClass::jumpAbsMs(MilliSeconds pos_ms)
{
	m->pipeline->seekAbsolute(pos_ms * GST_MSECOND);
}

void EngineClass::jumpRelMs(MilliSeconds ms)
{
	MilliSeconds new_time_ms = m->pipeline->positionMs() + ms;
	m->pipeline->seekAbsolute(new_time_ms * GST_MSECOND);
}

void EngineClass::jumpRel(double percent)
{
	m->pipeline->seekRelative(percent, m->md.durationMs() * GST_MSECOND);
}


void EngineClass::setCurrentPositionMs(MilliSeconds pos_ms)
{
	if(std::abs(m->currentPositionMs - pos_ms) >= EngineUtils::getUpdateInterval())
	{
		m->currentPositionMs = pos_ms;
		emit sigCurrentPositionChanged(pos_ms);
	}
}

void EngineClass::currentPositionChanged(MilliSeconds pos_ms)
{
	if(sender() == m->pipeline.get()) {
		this->setCurrentPositionMs(pos_ms);
	}
}


void EngineClass::setTrackReady(GstElement* src)
{
	if(m->pipeline->hasElement(src)) {
		emit sigTrackReady();
	}
}

void EngineClass::setTrackAlmostFinished(MilliSeconds time2go)
{
	if(sender() != m->pipeline.get()){
		return;
	}

	if( m->gaplessState == GaplessState::NoGapless ||
		m->gaplessState == GaplessState::AboutToFinish )
	{
		return;
	}

	spLog(Log::Develop, this) << "About to finish: " <<
		int(m->gaplessState) << " (" << time2go << "ms)";

	m->changeGaplessState(GaplessState::AboutToFinish);

	bool crossfade = GetSetting(Set::Engine_CrossFaderActive);
	if(crossfade) {
		m->pipeline->fadeOut();
	}

	emit sigTrackFinished();
}

void EngineClass::setTrackFinished(GstElement* src)
{
	if(m->pipeline->hasElement(src)) {
		emit sigTrackFinished();
	}

	if(m->otherPipeline && m->otherPipeline->hasElement(src))
	{
		spLog(Log::Debug, this) << "Old track finished";

		m->otherPipeline->stop();
		m->changeGaplessState(GaplessState::Playing);
	}
}

void EngineClass::setEqualizer(int band, int val)
{
	m->pipeline->setEqualizerBand(band, val);

	if(m->otherPipeline){
		m->otherPipeline->setEqualizerBand(band, val);
	}
}

MetaData Engine::Engine::currentTrack() const
{
	return m->md;
}

void EngineClass::setBufferState(int progress, GstElement* src)
{
	if(!Util::File::isWWW(m->md.filepath())){
		progress = -1;
	}

	else if(!m->pipeline->hasElement(src)){
		progress = -1;
	}

	emit sigBuffering(progress);
}

void EngineClass::gaplessChanged()
{
	Playlist::Mode plm = GetSetting(Set::PL_Mode);
	bool gapless =	(Playlist::Mode::isActiveAndEnabled(plm.gapless()) ||
					 GetSetting(Set::Engine_CrossFaderActive));

	if(gapless)
	{
		if(!m->otherPipeline) {
			m->otherPipeline = initPipeline("SecondPipeline");
		}

		m->changeGaplessState(GaplessState::Stopped);
	}

	else {
		m->changeGaplessState(GaplessState::NoGapless);
	}
}


void EngineClass::streamrecorderActiveChanged()
{
	bool is_active = GetSetting(Set::Engine_SR_Active);
	if(!is_active) {
		setStreamRecorderRecording(false);
	}
}

bool EngineClass::isStreamRecorderRecording() const
{
	bool sr_active = GetSetting(Set::Engine_SR_Active);
	return (sr_active && m->streamRecorder && m->streamRecorder->isRecording());
}

void EngineClass::setStreamRecorderRecording(bool b)
{
	if(b)
	{
		if(!m->streamRecorder) {
			m->streamRecorder = new StreamRecorder::StreamRecorder(this);
		}
	}

	if(!m->streamRecorder)	{
		return;
	}

	m->pipeline->record(b);

	if(m->streamRecorder->isRecording() != b){
		m->streamRecorder->record(b);
	}

	QString dst_file;
	if(b)
	{
		dst_file = m->streamRecorder->changeTrack(m->md);
		if(dst_file.isEmpty()){
			return;
		}
	}

	m->pipeline->setRecordingPath(dst_file);
}


void EngineClass::updateCover(GstElement* src, const QByteArray& data, const QString& mimetype)
{	
	if(m->pipeline->hasElement(src))
	{
		emit sigCoverDataAvailable(data, mimetype);
	}
}

void EngineClass::updateMetadata(const MetaData& md, GstElement* src)
{
	if(!m->pipeline->hasElement(src)){
		return;
	}

	if(!Util::File::isWWW( m->md.filepath() )) {
		return;
	}

	m->md = md;

	setCurrentPositionMs(0);

	emit sigMetadataChanged(m->md);

	if(isStreamRecorderRecording())
	{
		setStreamRecorderRecording(true);
	}
}

void EngineClass::updateDuration(GstElement* src)
{
	if(!m->pipeline->hasElement(src)){
		return;
	}

	MilliSeconds durationMs = m->pipeline->durationMs();
	MilliSeconds difference = std::abs(durationMs - m->md.durationMs());
	if(durationMs < 1000 || difference < 1999 || durationMs > 1500000000){
		return;
	}

	m->md.setDurationMs(durationMs);
	updateMetadata(m->md, src);

	emit sigDurationChanged(m->md);

	m->pipeline->checkPosition();
}

template<typename T>
T bitrateDiff(T a, T b){ return std::max(a, b) - std::min(a, b); }

void EngineClass::updateBitrate(Bitrate bitrate, GstElement* src)
{
	if( (!m->pipeline->hasElement(src)) ||
		(bitrate == 0) ||
		(bitrateDiff(bitrate, m->md.bitrate()) < 1000) )
	{
		return;
	}

	m->md.setBitrate(bitrate);

	emit sigBitrateChanged(m->md);
}

void EngineClass::setBroadcastEnabled(bool b)
{
	m->pipeline->setBroadcastingEnabled(b);
	if(m->otherPipeline) {
		m->otherPipeline->setBroadcastingEnabled(b);
	}
}

void EngineClass::setSpectrum(const SpectrumList& vals)
{
	m->spectrumValues = vals;
	emit sigSpectrumChanged();
}

Engine::SpectrumList Engine::Engine::spectrum() const
{
	return m->spectrumValues;
}

void EngineClass::setLevel(float left, float right)
{
	m->levelValues = {left, right};
	emit sigLevelChanged();
}

QPair<float, float> Engine::Engine::level() const
{
	return m->levelValues;
}

void Engine::Engine::setVisualizerEnabled(bool levelEnabled, bool spectrumEnabled)
{
	m->pipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
	if(m->otherPipeline){
		m->otherPipeline->setVisualizerEnabled(levelEnabled, spectrumEnabled);
	}
}

void EngineClass::error(const QString& error, const QString& element_name)
{
	QStringList msg{Lang::get(Lang::Error)};

	if(m->md.filepath().contains("soundcloud", Qt::CaseInsensitive))
	{
		msg << "Probably, Sayonara's Soundcloud limit of 15.000 "
			   "tracks per day is reached :( Sorry.";
	}

	if(error.trimmed().length() > 0){
		msg << error;
	}

	if(element_name.contains("alsa"))
	{
		msg << tr("You should restart Sayonara now") + ".";
	}

	stop();

	emit sigError(msg.join("\n\n"));
}
