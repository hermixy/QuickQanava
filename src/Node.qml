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
// This file is a part of the QuickQanava software library. Copyright 2015 Benoit AUTHEMAN.
//
// \file	Node.qml
// \author	benoit@destrat.io
// \date	2015 06 16
//-----------------------------------------------------------------------------

import QtQuick      2.7
import QuickQanava  2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Qan.AbstractNode {
    id: rectNode
    width: 110
    height: 50
    Qan.RectNodeTemplate {
        anchors.fill: parent
        node : rectNode
    }
}
