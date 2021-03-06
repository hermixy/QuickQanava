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
// This file is a part of the QuickQanava software library.
//
// \file	qanStyleSample.cpp
// \author	benoit@destrat.io
// \date	2016 02 09
//-----------------------------------------------------------------------------

// QuickContainers headers
#include "../../QuickContainers/src/QuickContainers.h"

// QuickQanava headers
#include "../../src/QuickQanava.h"
#include "./qanImgNode.h"

// Qt headers
#include <QPainter>

namespace qan { // ::qan

ImgNode::~ImgNode()
{
    qDebug() << "ImgNode::~ImgNode()";
    qDebug() << "\tobjectOwnership==Cpp =" << ( QQmlEngine::objectOwnership(this) == QQmlEngine::CppOwnership );
}

void    ImgNode::loadImageFromFile( QString fileName )
{
    QUrl url( fileName );
    if ( url.isValid() )
        fileName = url.toLocalFile();
    _image.load( fileName );
    emit imageChanged();
}

void    ImageItem::paint( QPainter* painter )
{
    if ( !_image.isNull() )
        painter->drawImage( QRectF( 0., 0., width(), height() ), _image );
}

ImageItem::~ImageItem()
{
    if ( QQmlEngine::objectOwnership( this ) == QQmlEngine::CppOwnership ) {
        //setParent( nullptr );
        setParentItem( nullptr );
    }
    qDebug() << "ImageItem::~ImageItem()";
    qDebug() << "\tobjectOwnership==Cpp = " << ( QQmlEngine::objectOwnership(this) == QQmlEngine::CppOwnership );
}

} // ::qan
