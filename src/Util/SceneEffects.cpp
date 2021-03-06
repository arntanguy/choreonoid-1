/*!
  @file
  @author Shin'ichiro Nakaoka
*/

#include "SceneEffects.h"
#include "SceneNodeClassRegistry.h"

using namespace std;
using namespace cnoid;


SgPolygonDrawStyle::SgPolygonDrawStyle()
    : SgGroup(findClassId<SgPolygonDrawStyle>())
{
    polygonElements_ = Face;
    edgeColor_ << 0.4f, 0.4f, 0.4f;
    edgeWidth_ = 0.75;
    vertexColor_ << 1.0f, 1.0f, 0.9f;
    vertexSize_ = 5.0f;
}


SgPolygonDrawStyle::SgPolygonDrawStyle(const SgPolygonDrawStyle& org, CloneMap* cloneMap)
    : SgGroup(org, cloneMap),
      polygonElements_(org.polygonElements_),
      edgeColor_(org.edgeColor_),
      edgeWidth_(org.edgeWidth_),
      vertexColor_(org.vertexColor_),
      vertexSize_(org.vertexSize_)
{

}
    

Referenced* SgPolygonDrawStyle::doClone(CloneMap* cloneMap) const
{
    return new SgPolygonDrawStyle(*this, cloneMap);
}


SgTransparentGroup::SgTransparentGroup()
    : SgGroup(findClassId<SgTransparentGroup>())
{
    transparency_ = 0.5f;
}


SgTransparentGroup::SgTransparentGroup(const SgTransparentGroup& org, CloneMap* cloneMap)
    : SgGroup(org, cloneMap)
{
    transparency_ = org.transparency_;
}
    

Referenced* SgTransparentGroup::doClone(CloneMap* cloneMap) const
{
    return new SgTransparentGroup(*this, cloneMap);
}


SgFog::SgFog(int classId)
    : SgPreprocessed(classId)
{
    color_.setOnes();
    visibilityRange_ = 0.0f;
}


SgFog::SgFog()
    : SgFog(findClassId<SgFog>())
{

}


SgFog::SgFog(const SgFog& org)
    : SgPreprocessed(org)
{
    color_ = org.color_;
    visibilityRange_ = org.visibilityRange_;
}


Referenced* SgFog::doClone(CloneMap*) const
{
    return new SgFog(*this);
}


SgOutline::SgOutline(int classId)
    : SgGroup(classId)
{
    lineWidth_ = 1.0;
    color_ << 1.0, 0.0, 0.0;
}


SgOutline::SgOutline()
    : SgOutline(findClassId<SgOutline>())
{

}


Referenced* SgOutline::doClone(CloneMap*) const
{
    return new SgOutline(*this);
}


SgLightweightRenderingGroup::SgLightweightRenderingGroup()
    : SgGroup(findClassId<SgLightweightRenderingGroup>())
{

}


Referenced* SgLightweightRenderingGroup::doClone(CloneMap*) const
{
    return new SgLightweightRenderingGroup(*this);
}


namespace {

struct NodeTypeRegistration {
    NodeTypeRegistration() {
        SceneNodeClassRegistry::instance()
            .registerClass<SgPolygonDrawStyle, SgGroup>()
            .registerClass<SgTransparentGroup, SgGroup>()
            .registerClass<SgFog, SgPreprocessed>()
            .registerClass<SgOutline, SgGroup>()
            .registerClass<SgLightweightRenderingGroup, SgGroup>();
    }
} registration;

}
