/* ConvertPipeline.h */

/* Copyright 2011-2017  Lucio Carreras
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

#ifndef ConvertPipeline_H
#define ConvertPipeline_H

#include "LameBitrate.h"
#include "Components/Engine/AbstractPipeline.h"

namespace Pipeline
{
    class Convert :
            public Base
    {
        Q_OBJECT
    public:
        explicit Convert(Engine::Base* engine, QObject *parent=nullptr);
		~Convert() override;

        bool set_uri(gchar* uri) override;
        bool init(GstState state=GST_STATE_NULL) override;
        GstElement* get_source() const override;

        bool set_target_uri(gchar* uri);

    public slots:
        void play() override;
        void stop() override;

        void set_quality(LameBitrate quality);

    private:
        GstElement* _audio_src=nullptr;
        GstElement* _lame=nullptr;
        GstElement* _decoder=nullptr;
        GstElement* _audio_convert=nullptr;
        GstElement* _audio_sink=nullptr;
        GstElement* _resampler=nullptr;
        GstElement* _xingheader=nullptr;

    protected:
        bool create_elements() override;
        bool add_and_link_elements() override;
        bool configure_elements() override;
    };
}
#endif // ConvertPipeline_H
