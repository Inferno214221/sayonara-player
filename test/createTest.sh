#!/bin/bash

if [ $# -ne 2 ] ; then
	echo "Call $0 <dir> <testname>"
	exit 1
fi

DIRECTORY=$1
PARAM=$2
EXT=${PARAM##*.}

if [ $EXT = "cpp" ] ; then
	TESTNAME=${PARAM%.*}
else
	TESTNAME=${PARAM}
fi

FILENAME=${TESTNAME}.cpp
rm -f ${FILENAME}

cat <<EOT >> ${FILENAME}
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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
 
#include "SayonaraTest.h"

// access working directory with Test::Base::tempPath("somefile.txt");

class ${TESTNAME} : 
    public Test::Base
{
    Q_OBJECT

    public:
        ${TESTNAME}() :
            Test::Base("${TESTNAME}")
        {}

    private slots:
        void test();
};

void ${TESTNAME}::test()
{}

QTEST_GUILESS_MAIN(${TESTNAME})
#include "${TESTNAME}.moc"
EOT

echo "new_test(${FILENAME})" >> CMakeLists.txt
