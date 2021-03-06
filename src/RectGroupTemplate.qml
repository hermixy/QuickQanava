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
// \file	RectGroupTemplate.qml
// \author	benoit@destrat.io
// \date	2016 06 21
//-----------------------------------------------------------------------------

import QtQuick              2.7
import QtQuick.Controls     1.4
import QtQuick.Layouts      1.3
import QtGraphicalEffects   1.0

import QuickQanava 2.0      as Qan
import "qrc:/QuickQanava"   as Qan

Item {
    id: template
    x: 0;   y: 0
    width: Math.max( content.width, 75 )
    height: Math.max( content.height, 40 )

    property int    nameTextSize: 14
    property bool   nameTextBold: true
    property color  nameTextColor: "black"

    property color  backColor: Qt.rgba( 0.97, 0.97, 0.97, 1.0 )

    default property alias children : content.children
    property alias  content: content

    property alias  radius: groupBackground.radius
    property alias  border: groupBackground.border

    property var    group

    signal  groupClicked( var group, var p )
    signal  groupRightClicked( var group, var p )
    signal  groupDoubleClicked( var group, var p )

    RowLayout {
        x: 0
        y: -collapser.height
        Text {
            id: collapser
            padding: 4
            text: group.collapsed ? "+" : "-"
            font.pointSize: nameTextSize; font.bold: nameTextBold
            color: nameTextColor
            Rectangle {
                id: collapserHilight
                anchors.fill: parent
                visible: collapserArea.containsMouse
                color: Qt.rgba( 0., 0., 0., 0. ); radius: 3
                border.color: nameTextColor; border.width: 1
            }
            MouseArea {
                id: collapserArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: group.collapsed = !group.collapsed
            }
        }
        TextInput {
            id: nameText
            text: group.label
            font.pointSize: nameTextSize; font.bold: nameTextBold
            color: nameTextColor
            onAccepted: {
                if ( text.length == 0 )
                    text = group.label  // Do not allow empty labels
                else group.label = text
                focus = false;  // Release focus once the label has been edited
            }
            onEditingFinished: { focus = false; }
        }
        MouseArea {
            id: groupLabelDrapArea
            anchors.fill: parent
            enabled: group.draggable    // Enable dragging on group label
            drag.target: group
            preventStealing: true; propagateComposedEvents: true // Ensure event are forwarded to collapserArea
            onDoubleClicked: { nameText.forceActiveFocus(); nameText.selectAll() }
        }
    } // RowLayout: collapser + label
    Item {
        id: content
        x: 0; y: 0; z: 3
        visible: !group.collapsed
    }
    MouseArea {  // 20160328: Do not set as content child to avoid interferring with content.childrenRect
        id: dragArea
        z: 2
        anchors.fill: parent
        enabled: group.draggable && !group.collapsed
        drag.target: group
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        smooth: true
        onPressed: { template.groupClicked( group, Qt.point( mouse.x, mouse.y ) ) }
        onClicked: {
            if ( mouse.button === Qt.LeftButton )
                template.groupClicked( group, Qt.point( mouse.x, mouse.y ) )
            else if ( mouse.button === Qt.RightButton )
                template.groupRightClicked( group, Qt.point( mouse.x, mouse.y ) )
        }
        onDoubleClicked: {
            if ( mouse.button === Qt.LeftButton )
                template.groupDoubleClicked( group, Qt.point( mouse.x, mouse.y ) )
        }
    }
    Rectangle { // 20160328: Do not set as content child to avoid interferring with content.childrenRect
        id: groupBackground
        anchors.fill: content
        visible: !group.collapsed
        z: 1;
        color: backColor
        radius: 7
        border.width: 2
        border.color: Qt.darker( backColor, 1.2 )
    }

    // Emitted by qan::Group when node dragging start
    function onNodeDragEnter() { groupBackground.color = Qt.binding( function() { return Qt.darker( template.backColor, 1.05 ) } ) }
    // Emitted by qan::Group when node dragging ends
    function onNodeDragLeave() { groupBackground.color = Qt.binding( function() { return template.backColor } ) }
}
