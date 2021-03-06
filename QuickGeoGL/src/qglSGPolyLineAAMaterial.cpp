/*
    This file is part of QuickGeoGL library.

    Copyright (C) 2016 Benoit AUTHEMAN

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
// This file is a part of the QuickGeoGL software. Copyright (C) 2016 Benoit Autheman.
//
// \file	qglSGSGPolyLineAAMaterial.cpp
// \author	benoit@qanava.org
// \date	2015 11 11
//-----------------------------------------------------------------------------

// Qt headers
#include <QOpenGLFunctions>
#include <QSGEngine>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

// QuickGeoGL headers
#include "./qglSGPolyLineAAMaterial.h"

namespace qgl { // ::qgl

QOpenGLShader* SGPolyLineAAShader::_gsh = nullptr;
QSGMaterialType SGPolyLineAAMaterial::_type;

SGPolyLineAAShader::SGPolyLineAAShader( ) :
    QSGMaterialShader{ }
{
    setShaderSourceFile( QOpenGLShader::Vertex, ":/QuickGeoGL/qglPolyLineAAVsh.glsl");

    if ( _gsh == nullptr ) {
        _gsh = new QOpenGLShader( QOpenGLShader::Geometry );
        if ( !_gsh->compileSourceFile( ":/QuickGeoGL/qglPolyLineAAGsh.glsl" ) )
            qDebug( ) << "qgl::SGPolyLineAAShader::SGPolyLineAAShader(): Error: Geometry shader compilation fails: " << _gsh->log( );
    }
    if ( _gsh != nullptr &&
         !program()->addShader( _gsh ) )
        qDebug( ) << "qgl::SGPolyLineAAShader::SGPolyLineAAShader(): Error: Geometry shader could not be added to OGL program.";

    setShaderSourceFile( QOpenGLShader::Fragment, ":/QuickGeoGL/qglPolyLineAAFsh.glsl");
}

void	SGPolyLineAAShader::updateState(const RenderState & state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial )
{
    QSGMaterialShader::updateState( state, newMaterial, oldMaterial );
    if ( program() == nullptr ||
         !program()->isLinked( ) ) {
        qDebug() << "qgl::SGPolyLineAAShader::updateState(): Error: Material shader is not linked.";
        return;
    }
    if ( state.isMatrixDirty( ) ) {
        program()->setUniformValue( _combMatrixId, state.combinedMatrix( ) );
        program()->setUniformValue( _mvMatrixId, state.modelViewMatrix( ) );
    }
    if ( state.isOpacityDirty( ) )
        program()->setUniformValue( _opacityId, state.opacity( ) );

    auto m = static_cast< SGPolyLineAAMaterial* >( newMaterial );
    if ( m != nullptr ) {
        program()->setUniformValue( _colorId, m->getColor( ) );
        program()->setUniformValue( _widthId, ( GLfloat )m->getWidth( ) );
    }
}

void SGPolyLineAAShader::initialize( )
{
    QSGMaterialShader::initialize();
    if ( program() == nullptr ||
         !program()->isLinked( ) ) {
        qDebug() << "qgl::SGLineAAShader::initialize(): Error: Material shader is not linked.";
        return;
    }
    _combMatrixId = program( )->uniformLocation( "combmatrix" );
    _mvMatrixId = program( )->uniformLocation( "mvmatrix" );
    _opacityId = program( )->uniformLocation( "opacity" );
    _colorId = program( )->uniformLocation( "color" );
    _widthId = program( )->uniformLocation( "width" );
}
//-----------------------------------------------------------------------------

} // ::qgl
