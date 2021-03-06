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
// \file	qanStyleManager.cpp
// \author	benoit@destrat.io
// \date	2015 06 05
//-----------------------------------------------------------------------------

// QT headers
#include <QFont>
#include <QQuickItemGrabResult>
#include <QQmlEngine>

// Qanava headers
#include "./qanGraph.h"
#include "./qanStyleManager.h"

namespace qan { // ::qan

/* Interview Interface *///----------------------------------------------------
bool    StylesFilterModel::filterAcceptsColumn( int sourceColumn, const QModelIndex& sourceParent) const
{
    Q_UNUSED( sourceParent );
    return ( sourceColumn > 0 ? false : true );
}

bool    StylesFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex& sourceParent) const
{
    Q_UNUSED( sourceParent );
    if ( sourceRow < 0 )
        return false;

    ObjectVectorModel* styles = qobject_cast< ObjectVectorModel* >( sourceModel( ) );
    if ( styles != nullptr ) {
        qan::Style* style = qobject_cast< qan::Style* >( styles->at( sourceRow ) );
        if ( style != nullptr &&
             style->getTarget( ) == _target )
            return true;
    }
    return false;
}

qan::Style* StylesFilterModel::getStyleAt( int styleIndex )
{
    if ( styleIndex < 0 )
        return nullptr;

    ObjectVectorModel* styles = qobject_cast<ObjectVectorModel*>( sourceModel() );
    if ( styles != nullptr ) {
        QModelIndex sourceIndex = mapToSource( index( styleIndex, 0 ) );
        if ( sourceIndex.isValid() ) {
            const auto style = styles->at( sourceIndex.row() );
            if ( style != nullptr )
                QQmlEngine::setObjectOwnership( style, QQmlEngine::CppOwnership );
            return qobject_cast< qan::Style* >( style );
        }
    }
    return nullptr;
}

int     StylesFilterModel::getStyleIndex( qan::Style* style )
{
    if ( style == nullptr )
        return -1;

    const auto styles = qobject_cast<ObjectVectorModel*>( sourceModel() );
    if ( styles != nullptr ) {
        int srcStyleIndex = styles->indexOf( style );
        QModelIndex dstStyleIndex = mapFromSource( styles->index( srcStyleIndex ) );
        if ( dstStyleIndex.isValid() )
            return dstStyleIndex.row();
    }
    return -1;
}

bool    StylesFilterModel::hasStyle( QString styleName )
{
    if ( styleName.isEmpty() )
        return false;

    const auto styles = qobject_cast<ObjectVectorModel* >( sourceModel() );
    if ( styles != nullptr ) {
        for ( const auto& p : qAsConst( *styles ) ) {
            qan::Style* s = qobject_cast< qan::Style* >( p );
            if ( s != nullptr &&
                 s->getName( ) == styleName )
                return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------


/* Style Object Management *///------------------------------------------------
StyleManager::StyleManager( QObject* parent ) :
    QObject( parent )
{
    _styles.setItemDisplayRole( QStringLiteral("name") ); // Use 'name' property for abstract list model display role
}

StyleManager::~StyleManager( )
{
    // Styles destroyed in qvs::PropertiesList::~PropertiesList
    for ( const auto& model : _targetModelMap )
        delete model;
    // _targetModelMap model maps will be destroyed by their parent QObject.
    _targetModelMap.clear();
    for ( const auto& defaultStyle : _defaultNodeStyles )
        delete defaultStyle;
    _defaultNodeStyles.clear();
    _defaultEdgeStyles.clear();
    _styles.clear();
}

void    StyleManager::clear()
{
    _styles.clear( true );
    for ( const auto& model : _targetModelMap )
        delete model;
    _targetModelMap.clear();
    _defaultNodeStyles.clear();
    _defaultEdgeStyles.clear();
}
//-----------------------------------------------------------------------------

/* Style Management *///-------------------------------------------------------
void    StyleManager::generateDefaultStyles()
{
    qan::NodeStyle* nodeStyle = createNodeStyle( QStringLiteral("default node") );
    qan::EdgeStyle* edgeStyle = createEdgeStyle( QStringLiteral("default edge") );
    if ( nodeStyle != nullptr )
        setDefaultNodeStyle( QStringLiteral("qan::Node"), nodeStyle );
    if ( edgeStyle != nullptr )
        setDefaultEdgeStyle( QStringLiteral("qan::Edge"), edgeStyle );
}

Style*  StyleManager::createStyle( QString styleName, QString targetName, QString metaTarget )
{
    if ( styleName.isEmpty( ) )
        return nullptr;

    Style* style = new qan::Style( styleName, targetName, metaTarget, this );
    _styles.append( style );
    if ( style != nullptr )
        QQmlEngine::setObjectOwnership( style, QQmlEngine::CppOwnership );
    return style;
}

qan::NodeStyle* StyleManager::createNodeStyle( QString styleName, QString targetName )
{
    if ( styleName.isEmpty() )
        return nullptr;

    NodeStyle* style = new qan::NodeStyle( styleName, targetName, this );
    _styles.append( style );
    if ( style != nullptr )
        QQmlEngine::setObjectOwnership( style, QQmlEngine::CppOwnership );
    return style;
}

qan::EdgeStyle* StyleManager::createEdgeStyle( QString styleName, QString targetName )
{
    if ( styleName.isEmpty() )
        return nullptr;

    EdgeStyle* style = new qan::EdgeStyle( styleName, targetName, this );
    _styles.append( style );
    if ( style != nullptr )
        QQmlEngine::setObjectOwnership( style, QQmlEngine::CppOwnership );
    return style;
}

Style*  StyleManager::duplicateStyle( QString styleName, QString duplicatedStyleName )
{
    if ( styleName.isEmpty() )
        return nullptr;

    // Find the existing style
    qan::Style* style = findStyleByName( styleName );
    qan::Style* duplicatedStyle = nullptr;

    if ( style != nullptr ) {
        if ( duplicatedStyleName.isEmpty() )
            duplicatedStyleName = styleName + QString( " 2" );
        duplicatedStyle = style->duplicate( duplicatedStyleName, this );
    }
    if ( duplicatedStyle != nullptr ) {
        _styles.append( duplicatedStyle );
        QQmlEngine::setObjectOwnership( style, QQmlEngine::CppOwnership );
    }
    return duplicatedStyle;
}

bool    StyleManager::removeStyle( QString styleName )
{
    if ( styleName.isEmpty( ) )
        return false;
    qan::Style* style = findStyleByName( styleName );
    if ( style != nullptr ) {
        _styles.remove( style );
        return true;
    }
    return false;
}

Style*  StyleManager::findStyleByName( QString styleName ) const
{
    if ( styleName.isEmpty( ) )
        return nullptr;
    for ( const auto object : qAsConst(_styles) ) {   // Slow, but there shouldn't be so many styles...
        const auto style = qobject_cast< const qan::Style* >( object );
        if ( style != nullptr &&
             style->getName( ) == styleName )
            return const_cast<Style*>(style);
    }
    return nullptr;
}

Style*  StyleManager::findStyleByTarget( QString targetName ) const
{
    if ( targetName.isEmpty( ) )
        return nullptr;

    for ( const auto properties : qAsConst(_styles) ) {       // Slow, but there shouldn't be so many styles...
        const auto style = qobject_cast< const qan::Style* >( properties );
        if ( style != nullptr &&
             style->getTarget( ) == targetName )
            return const_cast<Style*>(style);
    }
    return nullptr;
}

QList< qan::Style* >     StyleManager::getStylesByTarget( QString targetName ) const
{
    if ( targetName.isEmpty() )
        return QList<qan::Style*>{};

    QList< qan::Style* > styles;
    for ( const auto object : qAsConst(_styles) ) {       // Slow, but there shouldn't be so many styles...
        const auto style = qobject_cast< const qan::Style* >( object );
        if ( style != nullptr &&
             style->getTarget() == targetName )
            styles.append( const_cast<qan::Style*>(style) );
    }
    return styles;
}

void    StyleManager::setDefaultNodeStyle( QString targetName, qan::NodeStyle* defaultNodeStyle )
{
    _defaultNodeStyles.insert( targetName, defaultNodeStyle );
}

qan::Style*     StyleManager::getDefaultNodeStyle( QString targetName )
{
    if ( _defaultNodeStyles.contains( targetName ) )
        return _defaultNodeStyles.value( targetName, nullptr );
    return nullptr;
}

void    StyleManager::setDefaultEdgeStyle( QString targetName, qan::EdgeStyle* defaultEdgeStyle )
{
    _defaultEdgeStyles.insert( targetName, defaultEdgeStyle );
}

qan::Style*     StyleManager::getDefaultEdgeStyle( QString targetName )
{
    if ( _defaultEdgeStyles.contains( targetName ) )
        return _defaultEdgeStyles.value( targetName, nullptr );
    return nullptr;
}
//-----------------------------------------------------------------------------

/* Styles Model Management *///------------------------------------------------
QAbstractItemModel*     StyleManager::getStylesModelForTarget( QString target )
{
    if ( target.isEmpty() )
        return nullptr;
    QAbstractProxyModel* targetModel = static_cast< QAbstractProxyModel* >( _targetModelMap.value( target, nullptr ) );
    if ( targetModel == nullptr ) {
        targetModel = new StylesFilterModel{target};
        targetModel->setSourceModel( &_styles );
        _targetModelMap.insert( target, targetModel );
    }
    if ( targetModel != nullptr )
        QQmlEngine::setObjectOwnership( targetModel, QQmlEngine::CppOwnership );

    return targetModel;
}
//-----------------------------------------------------------------------------

} // ::qan

