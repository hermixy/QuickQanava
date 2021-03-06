/*
    This file is part of QuickContainers library.

    Copyright (C) 2016  Benoit AUTHEMAN

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickContainers library.
//
// \file    qcmContainerModelComposerWatcher.h
// \author  benoit@destrat.io
// \date    2016 11 28
//-----------------------------------------------------------------------------

#ifndef qcmContainerModelComposerWatcher_h
#define qcmContainerModelComposerWatcher_h

// Qt headers
#include <QObject>
#include <QAbstractListModel>
#include <QPointer>

// QuickContainers headers
#include "./qcmAbstractContainerModel.h"

namespace qcm { // ::qcm

/*! \brief Utility class used in model composer to watch source model signals notifications.
 *
 */
class ContainerModelComposerWatcher : public QObject
{
    Q_OBJECT
    /*! \name ContainerModelComposerWatcher Management *///--------------------
    //@{
public:
    ContainerModelComposerWatcher( AbstractContainerModel& target, QObject* parent = nullptr ) :
        QObject{ parent },
        _target{&target} { }
    virtual ~ContainerModelComposerWatcher() { }
    ContainerModelComposerWatcher( const ContainerModelComposerWatcher& ) = delete;
protected:
    QPointer<AbstractContainerModel>    _target{nullptr};

public:
    AbstractContainerModel* getM1( ) { return _m1.data(); }
    void                    setM1( AbstractContainerModel* m1 ) {
        if ( m1 != _m1 ) {
            if ( _m1 != nullptr ) {
                disconnectSource( _m1 );
                removeAllM1Items();
            }
            insertAllM1Items(m1);
            _m1 = m1;
            connectSource(m1);
            _target->emitItemCountChanged();
        }
    }
    AbstractContainerModel* getM2( ) { return _m2.data(); }
    void                    setM2( AbstractContainerModel* m2 ) {
        if ( m2 != _m2 ) {
            if ( _m2 != nullptr ) {
                disconnectSource(_m2);
                removeAllM2Items();
            }
            appendAllM2Items(m2);
            _m2 = m2;
            connectSource(m2);
            _target->emitItemCountChanged();
        }
    }
protected:
    QPointer<AbstractContainerModel>    _m1;
    QPointer<AbstractContainerModel>    _m2;

    virtual void    insertAllM1Items(AbstractContainerModel* model) = 0;
    virtual void    appendAllM2Items(AbstractContainerModel* model) = 0;
    virtual void    removeAllM1Items() = 0;
    virtual void    removeAllM2Items() = 0;

protected slots:
    virtual void    onSourceModelAboutToBeReset();
    virtual void    onSourceModelReset();
    virtual void    onSourceDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                         const QVector<int> &roles = QVector<int> ());
    virtual void    onSourceRowsInserted(const QModelIndex &parent, int first, int last);
    virtual void    onSourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    //void    rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

protected:
    void    connectSource(QAbstractListModel* source);

    //! Disconnect a source from this composed model.
    void    disconnectSource(QAbstractListModel* source);
    //@}
    //-------------------------------------------------------------------------
};

template < typename ContainerModelComposer, typename M1 = ContainerModelComposer, typename M2 = ContainerModelComposer >
class ContainerModelComposerWatcherImpl : public qcm::ContainerModelComposerWatcher
{
public:
    ContainerModelComposerWatcherImpl( ContainerModelComposer& target,
                                   QObject* parent = nullptr ) :
        qcm::ContainerModelComposerWatcher{ target, parent },
        _implTarget{&target} { }
    virtual ~ContainerModelComposerWatcherImpl() { }
    ContainerModelComposerWatcherImpl( const ContainerModelComposerWatcherImpl<ContainerModelComposer, M1, M2>& ) = delete;

protected:
    QPointer<ContainerModelComposer>    _implTarget{nullptr};

    using Base = qcm::ContainerModelComposerWatcher;

    virtual void    insertAllM1Items( AbstractContainerModel* m1 ) override {
        auto m2Model = static_cast<ContainerModelComposer*>( _m2.data() );
        if ( m2Model == nullptr ||          // Append all m1 content to this composer model using a fast append
             m2Model->size() == 0 ) {       // If m2 has still not been set or m2 is empty
            auto m1Model = static_cast<ContainerModelComposer*>( m1 );
            if ( m1Model != nullptr )
                for ( const auto& m1Item : qAsConst(*m1Model) )
                    _implTarget->append( m1Item );
        } else {        // m2 has been set, or there already is content in the list, use prepend()
            int i{0};
            auto m1Model = static_cast<ContainerModelComposer*>( m1 );
            if ( m1Model != nullptr )
                for ( const auto& m1Item : qAsConst(*m1Model) )
                    _implTarget->insert( m1Item, i++ );
        }
    }
    virtual void    appendAllM2Items( AbstractContainerModel* m2 ) override {
        auto m2Model = static_cast<ContainerModelComposer*>( m2 );
        if ( m2Model != nullptr ) {
            for ( const auto& m2Item : qAsConst(*m2Model) ) // Append all m2 content to this composer model
                _implTarget->append( m2Item );
        }
    }

    virtual void    removeAllM1Items() override {
        auto m1Model = static_cast<ContainerModelComposer*>( _m1.data() );
        if ( m1Model != nullptr )
            for ( const auto& m1Item : qAsConst(*m1Model) )
                _implTarget->remove( m1Item );
    }
    virtual void    removeAllM2Items() override {
        auto m2Model = static_cast<ContainerModelComposer*>( _m2.data() );
        if ( m2Model != nullptr )
            for ( const auto& m2Item : qAsConst(*m2Model) )
                _implTarget->remove( m2Item );
    }

    virtual void    onSourceModelAboutToBeReset() override {
        qDebug() << "ContainerModelComposerImpl<>::onSourceModelAboutToBeReset()";
        auto sourceModel = qobject_cast<QAbstractListModel*>(sender());
        if ( sourceModel == nullptr )
            return;
        if ( _m1 &&                     // When m1 is about to reset, remove all m1 content from container content, idem for m2
             sourceModel == _m1 ) {
            removeAllM1Items();
        } else if ( _m2 &&
                    sourceModel == _m2 ) {
            removeAllM2Items();
        }
    }
    virtual void    onSourceModelReset() override { /* empty */ }
    virtual void    onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                const QVector<int> &roles = QVector<int> ()) override
    {
        Base::onSourceDataChanged( topLeft, bottomRight, roles );
    }

    virtual void    onSourceRowsInserted(const QModelIndex &parent, int first, int last) override
    {
        Base::onSourceRowsInserted(parent, first, last);
        qDebug() << "ContainerModelComposerImpl<>::onSourceRowsInserted()";
        qDebug() << "\t_implTarget->mapRowFromSource( _m2, first )";
        if ( _target == nullptr )
            return;
        auto sourceModel = qobject_cast<QAbstractListModel*>(sender());
        if ( sourceModel == nullptr )
            return;
        if ( _m1 &&
             sourceModel == _m1 ) {
            //typename ContainerModelComposer::Item_type item = static_cast<typename ContainerModelComposer*>( _m1.data() )->at( first ) ;
            auto m1Item = static_cast<ContainerModelComposer*>( _m1.data() )->at( first );
            _implTarget->insert( m1Item, first );
        } else if ( _m2 &&
                    sourceModel == _m2 ) {
            typename ContainerModelComposer::Item_type item = static_cast<ContainerModelComposer*>( _m2.data() )->at( first );
            _implTarget->insert( item, _implTarget->mapRowFromSource( _m2, first ) );
        }
    }

    virtual void    onSourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last) override
    {
        Base::onSourceRowsAboutToBeRemoved(parent, first, last);
        if ( _target == nullptr )
            return;
        auto sourceModel = qobject_cast<QAbstractListModel*>(sender());
        if ( sourceModel == nullptr )
            return;
        if ( _m1 &&
             sourceModel == _m1 ) {
            typename ContainerModelComposer::Item_type item = static_cast<ContainerModelComposer*>( _m1.data() )->at( first );
            _implTarget->remove( item );
        } else if ( _m2 &&
                    sourceModel == _m2 ) {
            typename ContainerModelComposer::Item_type item = static_cast<ContainerModelComposer*>( _m2.data() )->at( first );
            _implTarget->remove( item );
        }
    }
};

} // ::qcm

#endif // qcmContainerModelComposerWatcher_h

