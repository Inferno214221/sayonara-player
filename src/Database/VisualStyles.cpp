/* DatabaseVisStyles.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "Database/Query.h"
#include "Database/VisualStyles.h"

#include "Gui/Plugins/Engine/VisualStyleTypes.h"
#include "Utils/Utils.h"

#include <QColor>

using DB::VisualStyles;
using DB::Module;

VisualStyles::VisualStyles(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id) {}

VisualStyles::~VisualStyles() = default;

QString col2String(const QColor& col)
{
	QString str;
	str = QString::number(col.red()) + "," +
	QString::number(col.green()) + "," +
	QString::number(col.blue()) + "," +
	QString::number(col.alpha());
	return str;
}


bool colFromString(const QString& str, QColor& c)
{
	QStringList colors = str.split(",");

	if(colors.size() < 3) {
		return false;
	}

	c.setRed(colors[0].toInt());
	c.setGreen(colors[1].toInt());
	c.setBlue(colors[2].toInt());

	if(colors.size() == 4){
		c.setAlpha(colors[3].toInt());
	}

	else{
		c.setAlpha(255);
	}

	return true;
}

QList<RawColorStyle> VisualStyles::get_raw_color_styles()
{
	QList<RawColorStyle> ret_val;

	Query q(this);
	q.prepare("SELECT * FROM VisualStyles;" );

	if(!q.exec()) {
		q.show_error("Could not fetch color styles");
		return ret_val;
	}

	while(q.next()) {
		RawColorStyle rcs;

		rcs.col_list.name = q.value(0).toString();
		QColor col1, col2, col3, col4;
		bool col3v, col4v;
		colFromString(q.value(1).toString(), col1);
		colFromString(q.value(2).toString(), col2);
		col3v = colFromString(q.value(3).toString(), col3);
		col4v = colFromString(q.value(4).toString(), col4);


		rcs.col_list.colors << col1;
		rcs.col_list.colors << col2;
		if(col3v) rcs.col_list.colors << col3;
		if(col4v) rcs.col_list.colors << col4;
		rcs.n_bins_spectrum = q.value(5).toInt();
		rcs.rect_height_spectrum = q.value(6).toInt();
		rcs.n_fading_steps_spectrum = q.value(7).toInt();
		rcs.hor_spacing_spectrum = q.value(8).toInt();
		rcs.ver_spacing_spectrum = q.value(9).toInt();
		rcs.rect_width_level = q.value(10).toInt();
		rcs.rect_height_level = q.value(11).toInt();
		rcs.hor_spacing_level = q.value(12).toInt();
		rcs.ver_spacing_level = q.value(13).toInt();
		rcs.n_fading_steps_level = q.value(14).toInt();

		ret_val << rcs;
	}

	return ret_val;
}


bool VisualStyles::insert_raw_color_style_to_db(const RawColorStyle& rcs)
{
	if(raw_color_style_exists(rcs.col_list.name))
		return update_raw_color_style(rcs);

	QString col_str;
	for(int i=0; i<4; i++) {
		col_str += ":col" + QString::number(i + 1) + ", ";
	}

	Query q(this);
	QString sql_str;
	sql_str = "INSERT INTO VisualStyles VALUES ("
			":name, "
			+ col_str +
			":n_bins_sp, "
			":rect_height_sp, "
			":fading_steps_sp, "
			":h_spacing_sp, "
			":v_spacing_sp, "
			":rect_width_lv, "
			":rect_height_lv, "
			":h_spacing_lv, "
			":v_spacing_lv, "
			":fading_steps_lv"
			")";

	q.prepare(sql_str);
	q.bindValue(":name", Util::cvt_not_null(rcs.col_list.name));
	q.bindValue(":col1", col2String(rcs.col_list.colors[0]));
	q.bindValue(":col2", col2String(rcs.col_list.colors[1]));

	if(rcs.col_list.colors.size() > 2) {
		q.bindValue(":col3", col2String(rcs.col_list.colors[2]));
	}

	else {
		q.bindValue(":col3", QString(""));
	}

	if(rcs.col_list.colors.size() > 3) {
		q.bindValue(":col4", col2String(rcs.col_list.colors[3]));
	}

	else q.bindValue(":col4", QString(""));

	q.bindValue(":n_bins_sp", rcs.n_bins_spectrum);
	q.bindValue(":rect_height_sp", rcs.rect_height_spectrum);
	q.bindValue(":fading_steps_sp", rcs.n_fading_steps_spectrum);
	q.bindValue(":h_spacing_sp", rcs.hor_spacing_spectrum);
	q.bindValue(":v_spacing_sp", rcs.ver_spacing_spectrum);
	q.bindValue(":rect_width_lv", rcs.rect_width_level);
	q.bindValue(":rect_height_lv", rcs.rect_height_level);
	q.bindValue(":h_spacing_lv", rcs.hor_spacing_level);
	q.bindValue(":v_spacing_lv", rcs.ver_spacing_level);
	q.bindValue(":fading_steps_lv", rcs.n_fading_steps_level);


	if(!q.exec()) {
		q.show_error("Could not insert style");
		return false;
	}

	return true;
}


bool VisualStyles::update_raw_color_style(const RawColorStyle& rcs)
{
	if(!raw_color_style_exists(rcs.col_list.name)) {
		return insert_raw_color_style_to_db(rcs);
	}

	QString col_str;
	for(int i=0; i < 4; i++) {
		col_str += "col" + QString::number(i + 1) + "=:col" + QString::number(i + 1) + ", ";
	}

	Query q(this);
	QString sql_str;
	sql_str = "UPDATE VisualStyles SET "

			+ col_str +
			"nBinsSpectrum=:n_bins_sp, "
			"rectHeightSpectrum=:rect_height_sp, "
			"fadingStepsSpectrum=:fading_steps_sp, "
			"horSpacingSpectrum=:h_spacing_sp, "
			"vertSpacingSpectrum=:v_spacing_sp, "
			"rectWidthLevel=:rect_width_lv, "
			"rectHeightLevel=:rect_height_lv, "
			"horSpacingLevel=:h_spacing_lv, "
			"verSpacingLevel=:v_spacing_lv, "
			"fadingStepsLevel=:fading_steps_lv"
			" WHERE name=:name";

	q.prepare(sql_str);
	q.bindValue(":name", Util::cvt_not_null(rcs.col_list.name));
	q.bindValue(":col1", Util::cvt_not_null(col2String(rcs.col_list.colors[0])));
	q.bindValue(":col2", Util::cvt_not_null(col2String(rcs.col_list.colors[1])));

	if(rcs.col_list.colors.size() > 2) {
		q.bindValue(":col3", col2String(rcs.col_list.colors[2]));
	}
	else {
		q.bindValue(":col3", QString(""));
	}

	if(rcs.col_list.colors.size() > 3) {
		q.bindValue(":col4", col2String(rcs.col_list.colors[3]));
	}
	else {
		q.bindValue(":col4", QString(""));
	}

	q.bindValue(":n_bins_sp", rcs.n_bins_spectrum);
	q.bindValue(":rect_height_sp", rcs.rect_height_spectrum);
	q.bindValue(":fading_steps_sp", rcs.n_fading_steps_spectrum);
	q.bindValue(":h_spacing_sp", rcs.hor_spacing_spectrum);
	q.bindValue(":v_spacing_sp", rcs.ver_spacing_spectrum);
	q.bindValue(":rect_width_lv", rcs.rect_width_level);
	q.bindValue(":rect_height_lv", rcs.rect_height_level);
	q.bindValue(":h_spacing_lv", rcs.hor_spacing_level);
	q.bindValue(":v_spacing_lv", rcs.ver_spacing_level);
	q.bindValue(":fading_steps_lv", rcs.n_fading_steps_level);


	if(!q.exec()) {
		q.show_error(QString("Could not update style ") + rcs.col_list.name);
		return false;
	}

	return true;
}


bool VisualStyles::delete_raw_color_style(QString name)
{
	Query q(this);
	q.prepare("DELETE FROM visualstyles WHERE name=:name;");
	q.bindValue(":name", Util::cvt_not_null(name));

	if(!q.exec()) {
		q.show_error(QString("Could not delete Raw color style ") + name);
		return false;
	}

	return true;
}


bool VisualStyles::raw_color_style_exists(QString name)
{
	Query q(this);
	q.prepare("SELECT * FROM visualstyles WHERE name=:name;");
	q.bindValue(":name", Util::cvt_not_null(name));

	if(!q.exec()) {
		q.show_error("Cannot check if raw color style exists");
		return false;
	}

	if(!q.next()){
		return false;
	}

	return true;
}

