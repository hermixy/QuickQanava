/*
    The MIT License (MIT)

    Copyright (c) 2016 Benoit AUTHEMAN

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

//-----------------------------------------------------------------------------
// This file is a part of the GTpo software library.
//
// \file	gtpoProtoSerializer.cpp
// \author	benoit@qanava.org
// \date	2016 02 10
//-----------------------------------------------------------------------------

#ifdef GTPO_HAVE_PROTOCOL_BUFFER

#include "gtpo.pb.h"

namespace gtpo { // ::gtpo

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::initProtocolBuffer() -> void
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::shutDownProtocolBuffer() -> void
{
    google::protobuf::ShutdownProtobufLibrary();
}

template < class GraphConfig >
ProtoSerializer< GraphConfig >::ProtoSerializer( std::string nodeDefaultName ,
                                                 std::string edgeDefaultName,
                                                 std::string groupDefaultName ) :
    gtpo::Serializer< GraphConfig >( ),
    _nodeDefaultName( nodeDefaultName ),
    _edgeDefaultName( edgeDefaultName ),
    _groupDefaultName( groupDefaultName )
{
    registerNodeOutFunctor( getNodeDefaultName(),
                            [=]( google::protobuf::Any* anyNodes,
                                const WeakNode& weakNode,
                                const ObjectIdMap& objectIdMap ){
            if ( anyNodes == nullptr )
                return;
            SharedNode node = weakNode.lock();
            if ( !node )
                return;
            gtpo::pb::GTpoNode pbNode;
            serializeGTpoNodeOut( weakNode, pbNode, objectIdMap );
            anyNodes->PackFrom( pbNode );
        } );
    registerNodeInFunctor( [=]( const google::protobuf::Any& anyNode,
                               Graph& graph,
                               IdObjectMap& idObjectMap ) -> WeakNode {
            if ( anyNode.Is< gtpo::pb::GTpoNode >() ) {
                gtpo::pb::GTpoNode pbNode;
                if ( anyNode.UnpackTo( &pbNode ) ) {
                    WeakNode weakNode = graph.createNode( getNodeDefaultName() );
                    SharedNode node = weakNode.lock();
                    if ( node ) {    // Feed the newly created node with PB node data
                        serializeGTpoNodeIn( pbNode, weakNode, idObjectMap );
                        return node;
                    }
                }
            }
            return WeakNode();
        } );

    registerEdgeOutFunctor( getEdgeDefaultName(),
                            []( google::protobuf::Any* anyEdges,
                                const WeakEdge& edge,
                                const ObjectIdMap& objectIdMap ) -> bool {
            if ( anyEdges == nullptr )
                return false;
            gtpo::pb::GTpoEdge pbEdge;
            int srcNodeId = -1;
            int dstNodeId = -1;
            try {
                SharedEdge sharedEdge = edge.lock();
                if ( !sharedEdge )
                    return false;
                int edgeId = -1;
                try {
                    edgeId = objectIdMap.at( sharedEdge.get() );
                } catch ( std::out_of_range ) { }
                pbEdge.set_edge_id( edgeId );
                SharedNode srcNode = sharedEdge->getSrc().lock();
                SharedNode dstNode = sharedEdge->getDst().lock();
                if ( !srcNode || !dstNode )
                    return false;
                srcNodeId = objectIdMap.at( srcNode.get() );
                dstNodeId = objectIdMap.at( dstNode.get() );
                if ( srcNodeId != -1 && dstNodeId != -1 ) {
                    pbEdge.set_src_node_id(srcNodeId);
                    pbEdge.set_dst_node_id(dstNodeId);
                    pbEdge.set_weight( GraphConfig::getEdgeWeight( sharedEdge.get() ) );
                    anyEdges->PackFrom( pbEdge );
                    return true;
                }
            } catch( std::out_of_range ) { return false; }
              catch( std::bad_weak_ptr ) { return false; }
            return false;
        } );

    registerEdgeInFunctor( [=](  const google::protobuf::Any& anyEdge,
                                Graph& graph,
                                IdObjectMap& idObjectMap ) -> void* {
            if ( anyEdge.Is< gtpo::pb::GTpoEdge >() ) {
                gtpo::pb::GTpoEdge pbEdge;
                if ( anyEdge.UnpackTo( &pbEdge ) ) {
                    try {
                        WeakEdge weakEdge;
                        void* sourceObject = idObjectMap.at( pbEdge.src_node_id() );
                        void* destinationObject = idObjectMap.at( pbEdge.dst_node_id() );
                        if ( sourceObject != nullptr && destinationObject != 0 ) {
                            WeakNode source       = reinterpret_cast< typename GraphConfig::Node* >( sourceObject )->shared_from_this();
                            WeakNode destination  = reinterpret_cast< typename GraphConfig::Node* >( destinationObject )->shared_from_this();
                            weakEdge = graph.createEdge( getEdgeDefaultName(), source, destination );
                            if ( !weakEdge.expired() ) {
                                SharedEdge edge = weakEdge.lock();
                                if ( edge ) {
                                    idObjectMap.insert( std::make_pair( pbEdge.edge_id(), edge.get() ) );
                                    GraphConfig::setEdgeWeight( edge.get(), pbEdge.weight() );
                                }
                                return edge.get();  // FIXME 20160213: return shared edge
                            }
                        }
                    } catch (...) { return nullptr; }
                }
            }
            return nullptr;
        } );

    registerGroupOutFunctor( getGroupDefaultName(),
                            [=]( google::protobuf::Any* anyGroups,
                                const WeakGroup& weakGroup,
                                const ObjectIdMap& objectIdMap ) -> bool {
            if ( anyGroups == nullptr )
                return false;
            SharedGroup group = weakGroup.lock();
            if ( !group )
                return false;
            gtpo::pb::GTpoGroup pbGroup;
            serializeGTpoGroupOut( weakGroup, pbGroup, objectIdMap );
            anyGroups->PackFrom( pbGroup );
            return true;
        } );
    registerGroupInFunctor( [=]( const google::protobuf::Any& anyGroup,
                               Graph& graph,
                               IdObjectMap& idObjectMap ) -> WeakGroup {
            if ( anyGroup.Is< gtpo::pb::GTpoGroup >() ) {
                gtpo::pb::GTpoGroup pbGroup;
                if ( anyGroup.UnpackTo( &pbGroup ) ) {
                    WeakGroup weakGroup = graph.createGroup( getGroupDefaultName() );
                    SharedGroup group = weakGroup.lock();
                    if ( group != nullptr ) {    // Feed the newly created group with PB group data
                        serializeGTpoGroupIn( pbGroup, weakGroup, idObjectMap );
                        return group;
                    }
                }
            }
            return WeakGroup();
        } );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::serializeOut( const Graph& graph,
                                                      std::ostream& os,
                                                      gtpo::IProgressNotifier* progressNotifier ) -> void
{
    gtpo::IProgressNotifier voidNotifier;
    if ( progressNotifier == nullptr )
        progressNotifier = &voidNotifier;
    progressNotifier->beginProgress();
    serializeOut( graph, os, *progressNotifier, nullptr, nullptr );
    progressNotifier->endProgress();
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::serializeOut( const Graph& graph,
                                                      std::ostream& os,
                                                      gtpo::IProgressNotifier& progressNotifier,
                                                      const ::google::protobuf::Message* user1,
                                                      const ::google::protobuf::Message* user2 ) -> void
{
    if ( getObjectIdMap().size() == 0 )
        generateObjectIdMap( graph );
    ObjectIdMap& objectIdMap = getObjectIdMap();

    gtpo::pb::GTpoGraph pbGraph;

    int serializedNodeCout = 0;

    progressNotifier.beginProgress();
    progressNotifier.setPhaseCount( 3 );
    progressNotifier.beginPhase( "Saving nodes" );
    int     primitive = 0;
    double  primitiveCount = static_cast< double >( graph.getNodes().size() );
    for ( auto& node: graph.getNodes() ) {  // Serialize nodes
        if ( !node->isSerializable() )   // Don't serialize control nodes
            continue;
        auto nodeOutFunctor = _nodeOutFunctors.find( node->getClassName() );
        if ( nodeOutFunctor != _nodeOutFunctors.end() ) {
            ( nodeOutFunctor->second )( pbGraph.add_nodes(), node, objectIdMap );
            ++serializedNodeCout;
            progressNotifier.setPhaseProgress( ++primitive / primitiveCount );
        } else
            std::cerr << "gtpo::ProtoSerializer::serializeOut(): no out serialization functor available for node class:" << node->getClassName() << std::endl;
    }

    int serializedEdgeCout = 0;
    progressNotifier.beginPhase( "Saving edges" );
    primitive = 0;
    primitiveCount = static_cast< double >( graph.getEdges().size() );
    for ( auto& edge: graph.getEdges() ) {  // Serialize edges
        auto edgeOutFunctor = _edgeOutFunctors.find( edge->getClassName() );
        if ( edgeOutFunctor != _edgeOutFunctors.end() ) {
            if ( ( edgeOutFunctor->second )( pbGraph.add_edges(), edge, objectIdMap ) ) {
                ++serializedEdgeCout;
                progressNotifier.setPhaseProgress( ++primitive / primitiveCount );
            }
        } else
            std::cerr << "gtpo::ProtoSerializer::serializeOut(): no out serialization functor available for edge class:" << edge->getClassName() << std::endl;
    }

    int serializedGroupCout = 0;
    progressNotifier.beginPhase( "Saving groups" );
    primitive = 0;
    primitiveCount = static_cast< double >( graph.getGroups().size() );
    for ( auto& group: graph.getGroups() ) {  // Serialize groups
        auto groupOutFunctor = _groupOutFunctors.find( group->getClassName() );
        if ( groupOutFunctor != _groupOutFunctors.end() ) {
            if ( ( groupOutFunctor->second )( pbGraph.add_groups(), group, objectIdMap ) ) {
                ++serializedGroupCout;
                progressNotifier.setPhaseProgress( ++primitive / primitiveCount );
            }
        } else
            std::cerr << "gtpo::ProtoSerializer::serializeOut(): no out serialization functor available for group class:" << group->getClassName() << std::endl;
    }

    pbGraph.set_node_count( ( int )graph.getNodeCount() );
    pbGraph.set_edge_count( ( int )graph.getEdges().size() );
    pbGraph.set_group_count( ( int )graph.getGroups().size() );

    if ( user1 != nullptr )
        pbGraph.mutable_user1()->PackFrom( *user1 );
    if ( user2 != nullptr )
        pbGraph.mutable_user2()->PackFrom( *user2 );

    // Write the new address book back to disk.
    if ( !pbGraph.SerializeToOstream( &os) )
        std::cerr << "gtpo::ProtoSerializer::serializeOut(): Protocol Buffer Error while writing to output stream." << std::endl;

    if ( serializedNodeCout != (int)graph.getNodeCount() ) // Report errors
        std::cerr << "gtpo::ProtoSerializer::serializeOut(): Only " << serializedNodeCout << " nodes serialized while there is " << graph.getNodeCount() << " nodes in graph" << std::endl;
    if ( serializedEdgeCout != (int)graph.getEdges().size() )
        std::cerr << "gtpo::ProtoSerializer::serializeOut(): Only " << serializedEdgeCout << " edges serialized while there is " << graph.getEdges().size() << " edges in graph" << std::endl;

    progressNotifier.endProgress();
}

template < class GraphConfig >
void    ProtoSerializer< GraphConfig >::serializeGTpoNodeOut( const WeakNode& weakNode, gtpo::pb::GTpoNode& pbNode, const ObjectIdMap& objectIdMap )
{
    if ( objectIdMap.size() == 0 ) {
        std::cerr << "ProtoSerializer<>::serializeGTpoNodeOut(): Warning: Method called with an empty object ID map." << std::endl;
        return;
    }
    SharedNode node = weakNode.lock();
    if ( node == nullptr )
        return;
    pbNode.set_label( GraphConfig::getNodeLabel( node.get() ) );
    pbNode.set_x( GraphConfig::getNodeX( node.get() ) );
    pbNode.set_y( GraphConfig::getNodeY( node.get() ) );
    pbNode.set_width( GraphConfig::getNodeWidth( node.get() ) );
    pbNode.set_height( GraphConfig::getNodeHeight( node.get() ) );
    try {
        pbNode.set_node_id( objectIdMap.at( node.get() ) );
    } catch( ... ) { pbNode.set_node_id( -1 ); }
}

template < class GraphConfig >
void    ProtoSerializer< GraphConfig >::serializeGTpoGroupOut( const WeakGroup& weakGroup, gtpo::pb::GTpoGroup& pbGroup, const ObjectIdMap& objectIdMap )
{
    if ( objectIdMap.size() == 0 ) {
        std::cerr << "ProtoSerializer<>::serializeGTpoGroupOut(): Warning: Method called with an empty object ID map." << std::endl;
        return;
    }
    SharedGroup group = weakGroup.lock();
    if ( group == nullptr )
        return;
    pbGroup.set_label( GraphConfig::getGroupLabel( group.get() ) );
    pbGroup.set_x( GraphConfig::getGroupX( group.get() ) );
    pbGroup.set_y( GraphConfig::getGroupY( group.get() ) );
    pbGroup.set_width( GraphConfig::getGroupWidth( group.get() ) );
    pbGroup.set_height( GraphConfig::getGroupHeight( group.get() ) );

    // FIXME serialize repeated node_ids

    try {
        pbGroup.set_group_id( objectIdMap.at( group.get() ) );
    } catch( ... ) { pbGroup.set_group_id( -1 ); }
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerNodeOutFunctor( std::string nodeClassName,
                                                                NodeOutFunctor nodeOutFunctor ) -> void
{
    _nodeOutFunctors.insert( std::make_pair( nodeClassName, nodeOutFunctor ) );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerEdgeOutFunctor( std::string edgeClassName,
                                                                EdgeOutFunctor edgeOutFunctor ) -> void
{
    _edgeOutFunctors.insert( std::make_pair( edgeClassName, edgeOutFunctor ) );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerGroupOutFunctor( std::string groupClassName,
                                                                 GroupOutFunctor groupOutFunctor ) -> void
{
    _groupOutFunctors.insert( std::make_pair( groupClassName, groupOutFunctor ) );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::generateObjectIdMap( const Graph& graph ) -> ObjectIdMap&
{
    _objectIdMap.clear();
    int id = 0;
    for ( auto& node: graph.getNodes() )
        _objectIdMap.insert( std::make_pair( node.get(), ++id ) );

    for ( auto& edge: graph.getEdges() )
        _objectIdMap.insert( std::make_pair( edge.get(), ++id ) );

    return _objectIdMap;
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::serializeIn( std::istream& is,
                                                     Graph& graph,
                                                     gtpo::IProgressNotifier* progressNotifier ) -> void
{
    gtpo::IProgressNotifier voidNotifier;
    if ( progressNotifier == nullptr )
        progressNotifier = &voidNotifier;
    progressNotifier->beginProgress();
    serializeIn< gtpo::pb::GTpoVoid >( is, graph, *progressNotifier, nullptr );
    progressNotifier->endProgress();
}

template < class GraphConfig >
template < class User1, class User2 >
auto    ProtoSerializer< GraphConfig >::serializeIn( std::istream& is,
                                                     Graph& graph,
                                                     gtpo::IProgressNotifier& progressNotifier,
                                                     User1* user1,
                                                     User2* user2 ) -> void
{
    progressNotifier.beginProgress();
    progressNotifier.setPhaseCount( 3 );
    progressNotifier.beginPhase( "Loading nodes" );

    IdObjectMap& idObjectMap = getIdObjectMap();
    idObjectMap.clear();
    int serializedNodeCout = 0;
    int serializedEdgeCout = 0;
    int serializedGroupCout = 0;
    gtpo::pb::GTpoGraph inGraph;
    if ( inGraph.ParseFromIstream( &is ) ) {
        for ( const google::protobuf::Any& anyNode : inGraph.nodes() ) {    // Serializing nodes in
            WeakNode node;
            for ( auto nodeInFunctor : _nodeInFunctors ) {
                node = nodeInFunctor( anyNode, graph, idObjectMap );
                if ( !node.expired() ) {
                    ++serializedNodeCout;
                    break;
                }
            }
            if ( !node.lock() ) {
                std::cerr << "gtpo::ProtoSerializer::serializeIn(): Warning: A Protocol Buffer node has not been serialized in successfuly." << std::endl;
                std::cerr << "\tProtocol Buffer Error:" << anyNode.type_url() << std::endl;
            }
        }
        if ( idObjectMap.size() > 0 ) {   // No need to start edge serialization if node id map is empty...
            progressNotifier.beginPhase( "Loading edges" );
            for ( const google::protobuf::Any& anyEdge : inGraph.edges() ) {    // Serializing edges in
                bool edgeSerialized = false;
                for ( auto edgeInFunctor : _edgeInFunctors ) {
                    if ( edgeInFunctor( anyEdge, graph, idObjectMap ) ) {
                        edgeSerialized = true;
                        ++serializedEdgeCout;
                        break;
                    }
                }
                if ( !edgeSerialized ) {
                    std::cerr << "gtpo::ProtoSerializer::serializeIn(): Warning: A Protocol Buffer edge has not been serialized in successfuly." << std::endl;
                    std::cerr << "\tProtocol Buffer Error:" << anyEdge.type_url() << std::endl;
                }
            }

            progressNotifier.beginPhase( "Loading groups" );
            for ( const google::protobuf::Any& anyGroup : inGraph.groups() ) {    // Serializing groups in
                bool groupSerialized = false;
                for ( auto groupInFunctor : _groupInFunctors ) {
                    WeakGroup serializedGroup = groupInFunctor( anyGroup, graph, idObjectMap );
                    if ( !serializedGroup.expired() ) {
                        groupSerialized = true;
                        ++serializedGroupCout;
                        break;
                    }
                }
                if ( !groupSerialized ) {
                    std::cerr << "gtpo::ProtoSerializer::serializeIn(): Warning: A Protocol Buffer group has not been serialized in successfuly." << std::endl;
                    std::cerr << "\tProtocol Buffer Error:" << anyGroup.type_url() << std::endl;
                }
            }
        }

        // Serialize optional user message
        if ( inGraph.mutable_user1() != nullptr && inGraph.user1().Is< User1 >() )
            inGraph.mutable_user1()->UnpackTo( user1 );
        // Serialize optional user message
        if ( inGraph.mutable_user2() != nullptr && inGraph.user2().Is< User1 >() )
            inGraph.mutable_user2()->UnpackTo( user2 );
    } else
        std::cerr << "gtpo::ProtoSerializer::serializeIn(): Protocol Buffer reports an error while trying to read input stream" << std::endl;

    // Report errors
    if ( serializedNodeCout != (int)inGraph.node_count() )
        std::cerr << "gtpo::ProtoSerializer::serializeIn(): Only " << serializedNodeCout << " nodes serialized while there is " << inGraph.node_count() << " nodes in serialized graph" << std::endl;
    if ( serializedEdgeCout != (int)graph.getEdges().size() )
        std::cerr << "gtpo::ProtoSerializer::serializeOut(): Only " << serializedEdgeCout << " edges serialized while there is " << inGraph.edge_count() << " edges in graph" << std::endl;

    progressNotifier.endProgress();
}

template < class GraphConfig >
void    ProtoSerializer< GraphConfig >::serializeGTpoNodeIn( const gtpo::pb::GTpoNode& pbNode,
                                                             WeakNode& weakNode,
                                                             IdObjectMap& idObjectMap )
{
    SharedNode node = weakNode.lock();
    if ( node ) {    // Feed the newly created node with PB node data
        idObjectMap.insert( std::make_pair( pbNode.node_id(), node.get() ) );
        GraphConfig::setNodeLabel( node.get(), pbNode.label() );
        GraphConfig::setNodeX( node.get(), pbNode.x() );
        GraphConfig::setNodeY( node.get(), pbNode.y() );
        GraphConfig::setNodeWidth( node.get(), pbNode.width() );
        GraphConfig::setNodeHeight( node.get(), pbNode.height() );
    }
}

template < class GraphConfig >
void    ProtoSerializer< GraphConfig >::serializeGTpoGroupIn( const gtpo::pb::GTpoGroup& pbGroup,
                                                              WeakGroup& weakGroup,
                                                              IdObjectMap& idObjectMap )
{
    SharedGroup group = weakGroup.lock();
    if ( group != nullptr ) {    // Feed the newly created group with PB group data
        idObjectMap.insert( std::make_pair( pbGroup.group_id(), group.get() ) );

        GraphConfig::setGroupLabel( group.get(), pbGroup.label() );
        GraphConfig::setGroupX( group.get(), pbGroup.x() );
        GraphConfig::setGroupY( group.get(), pbGroup.y() );
        GraphConfig::setGroupWidth( group.get(), pbGroup.width() );
        GraphConfig::setGroupHeight( group.get(), pbGroup.height() );
        for ( const google::protobuf::int32 groupNodeId : pbGroup.node_ids() ) {    // Serializing edges in
            if ( groupNodeId >= 0 ) {
                void* groupNode = idObjectMap.at( groupNodeId );
                if ( groupNode != nullptr )
                    group->insertNode( reinterpret_cast< typename GraphConfig::Node* >( groupNode )->shared_from_this() );
            }
        }
    }
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerNodeInFunctor( NodeInFunctor nodeInFunctor ) -> void
{
    _nodeInFunctors.push_back( nodeInFunctor );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerEdgeInFunctor( EdgeInFunctor edgeInFunctor ) -> void
{
    _edgeInFunctors.push_back( edgeInFunctor );
}

template < class GraphConfig >
auto    ProtoSerializer< GraphConfig >::registerGroupInFunctor( GroupInFunctor groupInFunctor ) -> void
{
    _groupInFunctors.push_back( groupInFunctor );
}

} // ::gtpo

#endif