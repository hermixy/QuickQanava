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
// This file is a part of the QuickQanava software. Copyright 2015 Benoit AUTHEMAN.
//
// \file	NodeRectTemplate.qml
// \author	benoit@destrat.io
// \date	2015 11 30
//-----------------------------------------------------------------------------

import QtQuick              2.7
import QtQuick.Layouts      1.3
import QtQuick.Controls     2.0
import QtGraphicalEffects   1.0

import QuickQanava          2.0 as Qan

/*! \brief Default template component for building a custom rectangular qan::Node item.
 *
 * Node with custom content definition using "templates" is described in \ref qanavacustom
 */
Item {
    id: template
    property         var    node: undefined
    default property alias  children : contentLayout.children

    onWidthChanged: { if ( node ) node.setDefaultBoundingShape() }
    onHeightChanged: { if ( node ) node.setDefaultBoundingShape() }
    Rectangle {
        id: background
        anchors.fill: parent    // Background follow the content layout implicit size
        radius: 2
        color: node.style.backColor
        border.color: node.style.borderColor;   border.width: node.style.borderWidth
        antialiasing: true
    }
    DropShadow {
        id: backgroundShadow
        anchors.fill: parent
        source: background
        horizontalOffset: node.style.shadowOffset.width
        verticalOffset: node.style.shadowOffset.height
        radius: 4; samples: 8
        color: node.style.shadowColor
        visible: node.style.hasShadow
        transparentBorder: true
    }
    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: background.radius / 2; spacing: 0
        visible: !labelEditor.visible
        Text {
            id: nodeLabel
            Layout.fillWidth: true
            Layout.fillHeight: contentLayout.children.length === 0
            Layout.preferredHeight: contentHeight
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            textFormat: Text.PlainText
            text: node.label
            font: node.style.labelFont
            horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
            maximumLineCount: 3 // Must be set, otherwise elide don't work and we end up with single line text
            elide: Text.ElideRight; wrapMode: Text.Wrap
        }
        Item {
            id: contentLayout
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true;
            Layout.fillHeight: true
            visible: contentLayout.children.length > 0  // Hide if the user has not added any content
        }
    }
    Connections {
        target: node
        onNodeDoubleClicked: labelEditor.visible = true
    }
    NodeLabelEditor {
        id: labelEditor
        anchors.fill: parent
        anchors.margins: background.radius / 2
        node: parent.node
        visible: false
    }
}
