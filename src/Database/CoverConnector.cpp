#include "CoverConnector.h"
#include "Query.h"
#include "Utils/Utils.h"

using DB::Query;

DB::Covers::Covers(const QString& connection_name, DbId db_id) :
	DB::Module(connection_name, db_id)
{}

DB::Covers::~Covers() {}

bool DB::Covers::exists(const QString& hash)
{
	Query q_check(this);
	QString query_check("SELECT id FROM covers WHERE hash = :hash;");
	q_check.prepare(query_check);
	q_check.bindValue(":hash", hash);
	if(!q_check.exec())
	{
		q_check.show_error("Cannot check cover");
		return false;
	}

	return q_check.next();
}

bool DB::Covers::get_cover(const QString& hash, QPixmap& pm)
{
	Query q(this);
	QString query = "SELECT data FROM covers WHERE hash = :hash;";
	q.prepare(query);
	q.bindValue(":hash", hash);
	if(!q.exec()){
		q.show_error("Cannot fetch cover");
		return false;
	}

	if(q.next())
	{
		QByteArray data = q.value(0).toByteArray();
		pm = ::Util::cvt_bytearray_to_pixmap(data);

		return true;
	}

	return false;
}

bool DB::Covers::set_cover(const QString& hash, const QPixmap& pm)
{
	if(hash.isEmpty()){
		return false;
	}

	QByteArray data = ::Util::cvt_pixmap_to_bytearray(pm);

	if(this->exists(hash))
	{
		Query q(this);
		QString query("UPDATE covers SET data=:data WHERE hash=:hash;");
		q.prepare(query);
		q.bindValue(":data", data);
		q.bindValue(":hash", hash);

		if(!q.exec()){
			q.show_error("Cannot update cover");
			return false;
		}

		return true;
	}

	else
	{
		Query q(this);
		QString query("INSERT INTO covers (hash, data) VALUES (:hash, :data)");
		q.prepare(query);
		q.bindValue(":data", data);
		q.bindValue(":hash", hash);

		if(!q.exec()){
			q.show_error("Cannot insert cover");
			return false;
		}

		return true;
	}
}

bool DB::Covers::get_all_covers(QMap<QString, QPixmap>& covers)
{
	covers.clear();

	Query q(this);
	QString query = "SELECT hash, data FROM covers;";
	q.prepare(query);
	if(!q.exec()){
		q.show_error("Cannot fetch all covers");
		return false;
	}

	while(q.next())
	{
		QString hash = q.value(0).toString();
		QByteArray data = q.value(1).toByteArray();

		covers[hash] = ::Util::cvt_bytearray_to_pixmap(data);
	}

	return true;
}
