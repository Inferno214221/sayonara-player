#include <Utils/EqualizerSetting.h>
#include "SayonaraTest.h"
#include "Components/Equalizer/Equalizer.h"
#include "Database/Equalizer.h"
#include "Database/Connector.h"
#include "Utils/Settings/Settings.h"

class EqualizerTest :
	public Test::Base
{
	Q_OBJECT

	private:
		DB::Connector* db;

	public:
		EqualizerTest() :
			Test::Base("EqualizerTest")
		{
			this->db = DB::Connector::instance();
			Settings::instance();
		}

	private slots:
		void load();
		void restoreDefaults();
		void rename();
		void saveAs();
		void remove();
		void changeValue();
};

void EqualizerTest::load()
{
	auto defaults = DB::Equalizer::factoryDefaults();

	QVERIFY(GetSetting(Set::Eq_Last) == 0);

	Equalizer eq;
	auto names = eq.names();
	QVERIFY(eq.currentIndex() == 0);
	QVERIFY(eq.names().size() == defaults.size());
	for(int i = 0; i < eq.names().size(); i++)
	{
		const auto& equalizer = eq.equalizerSetting(i);
		QVERIFY(equalizer.id() == i + 1);
		QVERIFY(!equalizer.name().isEmpty());
		QVERIFY(equalizer.values() == equalizer.defaultValues());
	}
}

void EqualizerTest::restoreDefaults()
{
	Equalizer eq;

	const auto defaults = DB::Equalizer::factoryDefaults();
	const auto names = eq.names();
	eq.deletePreset(0);
	eq.deletePreset(0);

	QVERIFY(eq.names().size() == names.size() - 2);
	auto success = db->equalizerConnector()->restoreFactoryDefaults();
	QVERIFY(success);

	Equalizer eq2;
	QVERIFY(eq2.names().size() == defaults.size());
}

void EqualizerTest::rename()
{
	Equalizer eq;

	QVERIFY(GetSetting(Set::Eq_Last) == 0);

	QVERIFY(!eq.names().contains("new name"));

	auto err = eq.renamePreset(1, "new name");
	QVERIFY(err == Equalizer::RenameError::NoError);
	QVERIFY(eq.equalizerSetting(1).name() == "new name");

	const auto oldName = eq.equalizerSetting(2).name();
	err = eq.renamePreset(2, "");
	QVERIFY(err == Equalizer::RenameError::EmptyName);
	QVERIFY(eq.equalizerSetting(2).name() == oldName);

	err = eq.renamePreset(2, "new name");
	QVERIFY(err == Equalizer::RenameError::NameAlreadyKnown);
	QVERIFY(eq.equalizerSetting(2).name() == oldName);

	err = eq.renamePreset(1000, "new name");
	QVERIFY(err == Equalizer::RenameError::InvalidIndex);
	QVERIFY(eq.equalizerSetting(2).name() == oldName);

	Equalizer eq2;
	QVERIFY(eq2.names().indexOf("new name") == 1);

	db->equalizerConnector()->restoreFactoryDefaults();
}

void EqualizerTest::saveAs()
{
	Equalizer eq;

	eq.setCurrentIndex(1);

	auto oldName = eq.currentSetting().name();
	const auto names = eq.names();
	QVERIFY(!eq.names().contains("sonstwas"));

	auto renameError = eq.saveCurrentEqualizerAs("sonstwas");
	QVERIFY(renameError == Equalizer::RenameError::NoError);
	QVERIFY(eq.currentSetting().name() == oldName);
	QVERIFY(eq.names().size() == names.size() + 1);
	QVERIFY(eq.names().last() == "sonstwas");

	auto lastEqualizer = eq.equalizerSetting(eq.names().size() - 1);
	QVERIFY(lastEqualizer.values() == eq.currentSetting().values());
	QVERIFY(
		lastEqualizer.defaultValues() == eq.currentSetting().defaultValues());

	eq.setCurrentIndex(2);
	oldName = eq.currentSetting().name();
	QVERIFY(eq.saveCurrentEqualizerAs("sonstwas") ==
	        Equalizer::RenameError::NameAlreadyKnown);
	QVERIFY(eq.saveCurrentEqualizerAs("") == Equalizer::RenameError::EmptyName);

	Equalizer eq2;
	QVERIFY(eq2.names().last() == "sonstwas");

	db->equalizerConnector()->restoreFactoryDefaults();
}

void EqualizerTest::remove()
{
	Equalizer eq;
	const auto names = eq.names();

	const auto name = eq.equalizerSetting(0).name();
	const auto nextName = eq.equalizerSetting(1).name();

	eq.deletePreset(0);
	QVERIFY(eq.names().size() == names.size() - 1);
	QVERIFY(eq.names().first() == names[1]);

	eq.deletePreset(100);
	QVERIFY(eq.names().size() == names.size() - 1);
	QVERIFY(eq.names().first() == names[1]);

	for(auto i = 0; i < 10; i++)
	{
		eq.deletePreset(0);
	}

	QVERIFY(eq.currentIndex() == 0);
	QVERIFY(eq.names().size() == 1);

	EqualizerSetting::ValueArray values;
	values.fill(0);

	Equalizer eq2;
	auto names2 = eq2.names();
	QVERIFY(eq2.names().size() == 1);
	QVERIFY(eq2.currentSetting().values() == values);

	db->equalizerConnector()->restoreFactoryDefaults();
}

void EqualizerTest::changeValue()
{
	Equalizer eq;
	eq.setCurrentIndex(0);
	eq.setGaussEnabled(false);
	const auto valueCount = int(eq.currentSetting().values().size());
	for(int i = 0; i < valueCount; i++)
	{
		eq.changeValue(0, i, i);
	}

	const auto expected = EqualizerSetting::ValueArray{0,1,2,3,4,5,6,7,8,9};
	QVERIFY(eq.currentSetting().values() == expected);

	Equalizer eq2;
	eq2.setCurrentIndex(0);
	QVERIFY(eq2.currentSetting().values() == expected);

	db->equalizerConnector()->restoreFactoryDefaults();
}

QTEST_GUILESS_MAIN(EqualizerTest)

#include "EqualizerTest.moc"
