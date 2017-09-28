#include "AGXVehicleContinuousTrack.h"
#include "AGXBody.h"
#include "AGXScene.h"

namespace cnoid {

class TrackListener : public agxSDK::StepEventListener
{
public:
    TrackListener( AGXVehicleContinuousTrack* track )
            : m_track( track )
    {
        setMask( POST_STEP );
    }

    virtual void post( const agx::TimeStamp& /*t*/ )
    {
        m_track->updateTrackState();
    }

private:
    AGXVehicleContinuousTrack*  m_track;
};


AGXVehicleContinuousTrack::AGXVehicleContinuousTrack(AGXVehicleContinuousTrackDevice* const device, AGXBody* const agxBody) : AGXBodyExtension(agxBody)
{
    m_device = device;
    AGXVehicleTrackDesc trackDesc;

    auto createWheel = [device, agxBody](const string& name,
        const agxVehicle::TrackWheel::Model model) -> agxVehicle::TrackWheelRef
    {
        // get agxlink
        AGXLinkPtr l_agxLink = agxBody->getAGXLink(name);
        if(!l_agxLink){
            std::cout << "failed to get agxlink by linkname at AGXVehicleCotinuousTrack." << std::endl;
            return nullptr;
        }
        // get raduis of wheel
        if(l_agxLink->getAGXGeometry()->getShapes().size() <= 0 ) return nullptr;
        agxCollide::Shape* shape = l_agxLink->getAGXGeometry()->getShapes()[0];
        agxCollide::Cylinder* l_cylinder = dynamic_cast<agxCollide::Cylinder*>(shape);
        if(!l_cylinder){
            std::cout << "failed to cast agxCollide::Cylinder." << std::endl;
            return nullptr;
        }
        // create rotate matrix
        Link* l_link = l_agxLink->getOrgLink();
        const Vector3& a = l_link->a();            // rotational axis
        const Vector3& u = device->getUpAxis();    // up axis
        const agx::Vec3 ny = agx::Vec3(a(0), a(1), a(2)).normal();
        const agx::Vec3 nz = agx::Vec3(u(0), u(1), u(2)).normal();
        if(ny * nz > 1e-6) return nullptr;   // Vectors maybe not orthogonal to each other
        const agx::Vec3 nx = ny.cross(nz);
        agx::OrthoMatrix3x3 rotation;
        rotation.setColumn(0, nx);
        rotation.setColumn(1, ny);
        rotation.setColumn(2, nz);
        agx::Quat q;
        rotation.get(q);

        AGXVehicleTrackWheelDesc l_desc;
        l_desc.rigidbody = l_agxLink->getAGXRigidBody();
        l_desc.model = model;
        l_desc.radius = l_cylinder->getRadius();
        l_desc.rbRelTransform.setRotate(l_desc.rigidbody->getRotation());
        l_desc.rbRelTransform.setRotate(rotation);
        return AGXObjectFactory::createVehicleTrackWheel(l_desc);
    };

    auto createWheels = [createWheel, &trackDesc](const vector<string>& names, const agxVehicle::TrackWheel::Model& model)
    {
        for(auto it : names){
            trackDesc.trackWheelRefs.push_back(createWheel(it, model));
        }
    };

    // Create sprocket, idler and roller wheels
    createWheels(device->getSprocketNames(), agxVehicle::TrackWheel::Model::SPROCKET);
    createWheels(device->getIdlerNames(), agxVehicle::TrackWheel::Model::IDLER);
    createWheels(device->getRollerNames(), agxVehicle::TrackWheel::Model::ROLLER);

    // Create track
    AGXVehicleContinuousTrackDeviceDesc desc;
    device->getDesc(desc);
    trackDesc.numberOfNodes = (agx::UInt)desc.numberOfNodes;
    trackDesc.nodeThickness = desc.nodeThickness;
    trackDesc.nodeWidth = desc.nodeWidth;
    trackDesc.nodeThickerThickness = desc.nodeThickerThickness;
    trackDesc.useThickerNodeEvery = desc.useThickerNodeEvery;
    trackDesc.nodeDistanceTension = desc.nodeDistanceTension;
    trackDesc.hingeCompliance = desc.hingeCompliance;
    trackDesc.stabilizingHingeFrictionParameter = desc.stabilizingHingeFrictionParameter;
    trackDesc.enableMerge = desc.enableMerge;
    trackDesc.numNodesPerMergeSegment = (agx::UInt)desc.numNodesPerMergeSegment;
    switch(desc.contactReductionLevel)
    {
        case 0:
            trackDesc.contactReduction =  agxVehicle::TrackInternalMergeProperties::ContactReduction::NONE;
            break;
        case 1:
            trackDesc.contactReduction =  agxVehicle::TrackInternalMergeProperties::ContactReduction::MINIMAL;
            break;
        case 2:
            trackDesc.contactReduction =  agxVehicle::TrackInternalMergeProperties::ContactReduction::MODERATE;
            break;
        case 3:
            trackDesc.contactReduction =  agxVehicle::TrackInternalMergeProperties::ContactReduction::AGRESSIVE;
            break;
        default:
            break;
    }
    m_track = AGXObjectFactory::createVehicleTrack(trackDesc);


    // Add to simulation
    getAssembly()->add(m_track);
    getAGXBody()->getAGXScene()->add(getAssembly());
    getAGXBody()->getAGXScene()->getSimulation()->add(new TrackListener(this));

    /* Set collision Group*/
    // 1. All links are member of body's collision group
    // 2. Collision b/w tracks and wheels must need -> remove wheels from body's collision
    // 3. Collision b/w wheels and body(except tracks) are not need -> create new group and join
    m_track->addGroup(getAGXBody()->getCollisionGroupName());
    std::stringstream bodyWheelCollision;
    bodyWheelCollision << "BodyWheelCollision" << generateUID() << std::endl;
    getAGXBody()->addCollisionGroupNameToDisableCollision(bodyWheelCollision.str());
    getAGXBody()->addCollisionGroupNameToAllLink(bodyWheelCollision.str());
    m_track->removeGroup(bodyWheelCollision.str());

    for(auto wheels : trackDesc.trackWheelRefs){
        agxCollide::GeometryRef geometry = wheels->getRigidBody()->getGeometries().front();
        geometry->removeGroup(getAGXBody()->getCollisionGroupName());
        geometry->addGroup(bodyWheelCollision.str());
    }

    /* Rendering */
    // Retrieve size and transform of node from track for graphic rendering
    // Limit only one geometry and one shape
    m_device->initialize();
    m_device->reserveTrackStateSize((unsigned int)m_track->nodes().size());
    for(auto node : m_track->nodes()){
        if(agxCollide::Shape* const shape = node->getRigidBody()
                ->getGeometries().front()->getShapes().front()){
            if(agxCollide::Box* const box = static_cast<agxCollide::Box*>(shape)){
                const agx::Vec3& e = box->getHalfExtents() * 2.0;
                Vector3 size(e[0], e[1], e[2]);
                const Position& pos = convertToPosition(node->getRigidBody()->getTransform());
                m_device->addTrackState(size, pos);
            }
        }
    }
    m_device->notifyStateChange();
}

void AGXVehicleContinuousTrack::updateTrackState() {
    for(size_t i = 0; i < m_track->nodes().size(); ++i){
        m_device->getTrackStates()[i].position =
            convertToPosition(m_track->nodes()[i]->getRigidBody()->getGeometries().front()->getTransform());
    }
    m_device->notifyStateChange();
}


}