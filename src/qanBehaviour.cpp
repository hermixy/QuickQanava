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
// \file	qanBehaviour.cpp
// \author	benoit@destrat.io
// \date	2016 04 04
//-----------------------------------------------------------------------------

// Qt headers
// Nil

// Qanava headers
#include "./qanBehaviour.h"

namespace qan { // ::qan

NodeBehaviour::NodeBehaviour( const std::string& name, QObject* parent ) :
    QObject( parent ),
    gtpo::NodeBehaviour< qan::Config >::NodeBehaviour( name )
{

}

void    NodeBehaviour::inNodeInserted( WeakNode& weakInNode ) noexcept
{
    SharedNode inNode = weakInNode.lock();
    if ( inNode != nullptr )
        inNodeInserted( *inNode );
}

void    NodeBehaviour::inNodeAboutToBeRemoved( WeakNode& weakInNode ) noexcept
{
    SharedNode inNode = weakInNode.lock();
    if ( inNode != nullptr )
        inNodeAboutToBeRemoved( *inNode );
}

void    NodeBehaviour::outNodeInserted( WeakNode& weakOutNode ) noexcept
{
    SharedNode outNode = weakOutNode.lock();
    if ( outNode != nullptr )
        outNodeInserted( *outNode );
}

void    NodeBehaviour::outNodeRemoved( WeakNode& weakOutNode ) noexcept
{
    SharedNode outNode = weakOutNode.lock();
    if ( outNode != nullptr )
        outNodeRemoved( *outNode );
}

void    NodeBehaviour::nodeModified( WeakNode& weakNode ) noexcept
{
    SharedNode node = weakNode.lock();
    if ( node != nullptr )
        nodeModified( *node );
}

} // ::qan
