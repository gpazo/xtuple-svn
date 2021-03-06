#
# OpenRPT report writer and rendering engine
# Copyright (C) 2001-2008 by OpenMFG, LLC
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# Please contact info@openmfg.com with any questions on this license.
#

include( ../global.pri )

TEMPLATE    = lib
CONFIG      += qt warn_on staticlib
QT          += xml sql
DEFINES     += MAKELIB

INCLUDEPATH = .

DESTDIR = ../lib
OBJECTS_DIR = tmp
MOC_DIR     = tmp
UI_DIR      = tmp

SOURCES = parameter.cpp \
          dbtools.cpp \
          xsqlquery.cpp \
          quuencode.cpp \
          login.cpp \
          loginOptions.cpp \
          xvariant.cpp \
          languageoptions.cpp \
          newvariant.cpp \
          booledit.cpp \
          doubleedit.cpp \
          intedit.cpp \
          listedit.cpp \
          stringedit.cpp \
          paramlistedit.cpp \
          parameteredit.cpp \
          parsexmlutils.cpp \
          querysource.cpp \
          reportpageoptions.cpp \
          memdbloader.cpp

          
HEADERS = parameter.h \
          dbtools.h \
          xsqlquery.h \
          quuencode.h \
          login.h \
          loginOptions.h \
          xvariant.h \
          languageoptions.h \
          newvariant.h \
          booledit.h \
          doubleedit.h \
          intedit.h \
          listedit.h \
          stringedit.h \
          paramlistedit.h \
          parameteredit.h \
          parsexmlutils.h \
          querysource.h \
          reportpageoptions.h \
          memdbloader.h

          
FORMS   =  newvariant.ui \
           booledit.ui \
           doubleedit.ui \
           intedit.ui \
           listedit.ui \
           stringedit.ui \
           paramlistedit.ui \
           login.ui \
           loginOptions.ui \
           parameteredit.ui

RESOURCES += ../OpenRPT/images/OpenRPTCommon.qrc
