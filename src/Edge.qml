/*
    This file is part of QuickQanava library.

    Copyright (C) 2008-2017 Benoit AUTHEMAN

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library. Copyright 2014 Benoit AUTHEMAN.
//
// \file	Edge.qml
// \author	benoit@destrat.io
// \date	2015 06 20
//-----------------------------------------------------------------------------

import QtQuick          2.7
import QtQuick.Layouts  1.3

import QuickQanava      2.0 as Qan
import QuickGeoGL           1.0 as Qgl
import "qrc:/QuickGeoGL"    1.0 as Qgl

Qan.Edge {
    id: arrowEdge
    property color  color: Qt.rgba(0,0,0,1)
    Qgl.Arrow {
        anchors.fill: parent
        id: arrow
        p1: arrowEdge.p1
        p2: arrowEdge.p2
        p2CapSize: arrowEdge.style ? arrowEdge.style.arrowSize : 4
        lineWidth: arrowEdge.style ? arrowEdge.style.lineWidth : 2
    }
}
