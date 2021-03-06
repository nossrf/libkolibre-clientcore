#!/bin/sh -e

## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-clientcore.
#
# Kolibre-clientcore is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-clientcore is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
#

#
# Setup parameters
#

SERVICE_URL=http://kados.kolibre.org/service.php
USERNAME=kolibre
PASSWORD=kolibre

KOLIBRE_PREFIX=$(sh kolibre_prefix.sh)

if [ -x /usr/bin/gdb ]; then
    PREFIX="libtool --mode=execute gdb --return-child-result -batch -x ${srcdir:-.}/run --args"
fi

#
# Setup data paths
#

DATA_PATH=userdata
test -d ${DATA_PATH} || mkdir ${DATA_PATH}

#
# We cannot use messages database from where it is installed.
# Therefore we need to copy it to DATA_PATH.
#

if ! [ -e ${DATA_PATH}/messages.db ]; then
    cp ${KOLIBRE_PREFIX}/share/libkolibre-narrator/messages.db ${DATA_PATH}/messages.db
fi

#
# Axis2c will complain if it can't find the path where it is installed.
# Override default data path /cache with KOLIBRE_DATA_PATH
#

BOOKMARK_DIR=${DATA_PATH} KOLIBRE_DATA_PATH=${DATA_PATH} ${PREFIX} ./KolibreSampleClient $SERVICE_URL $USERNAME $PASSWORD -c log4cxx.conf
