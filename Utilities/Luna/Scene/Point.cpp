#include "Precompile.h"
#include "Point.h"
#include "Transform.h"
#include "Core/Enumerator.h"
#include "Application/UI/ImageManager.h"

using namespace Luna;

LUNA_DEFINE_TYPE( Luna::Point );

void Point::InitializeType()
{
  Reflect::RegisterClass< Luna::Point >( "Luna::Point" );
  Enumerator::InitializePanel( "Point", CreatePanelSignature::Delegate( &Point::CreatePanel ) );
}

void Point::CleanupType()
{
  Reflect::UnregisterClass< Luna::Point >();
}

Point::Point(Luna::Scene* scene, Content::Point* data) 
: Luna::HierarchyNode(scene, data)
{
}

Point::~Point()
{
}

i32 Point::GetImageIndex() const
{
  return Nocturnal::GlobalImageManager().GetImageIndex( "point.png" );
}

std::string Point::GetApplicationTypeName() const
{
  return "Point";
}

const Math::Vector3& Point::GetPosition() const
{
  return GetPackage<Content::Point>()->m_Position;
}

void Point::SetPosition( const Math::Vector3& value )
{
  Content::Point* point = GetPackage<Content::Point>();
  if ( point->m_Position != value )
  {
    point->m_Position = value;
    Dirty();
  }
}

f32 Point::GetPositionX() const
{
  return GetPosition().x;
}

void Point::SetPositionX( f32 value )
{
  Math::Vector3 pos = GetPosition();
  pos.x = value;
  SetPosition( pos );
}

f32 Point::GetPositionY() const
{
  return GetPosition().y;
}

void Point::SetPositionY( f32 value )
{
  Math::Vector3 pos = GetPosition();
  pos.y = value;
  SetPosition( pos );
}

f32 Point::GetPositionZ() const
{
  return GetPosition().z;
}

void Point::SetPositionZ( f32 value )
{
  Math::Vector3 pos = GetPosition();
  pos.z = value;
  SetPosition( pos );
}

void Point::ConnectManipulator(ManiuplatorAdapterCollection *collection)
{
  switch( collection->GetMode() )
  {
  case ManipulatorModes::Translate:
    {
      collection->AddManipulatorAdapter( new PointTranslateManipulatorAdapter( this ) );
      break;
    }

  default:
    {
      break;
    }
  }
}

bool Point::ValidatePanel( const std::string& name )
{
  if ( name == "Point" )
  {
    return true;
  }

  return __super::ValidatePanel( name );
}

void Point::CreatePanel( CreatePanelArgs& args )
{
  args.m_Enumerator->PushPanel( "Point", true);
  {
    args.m_Enumerator->PushContainer();
    args.m_Enumerator->AddLabel( "Position" );
    args.m_Enumerator->AddValue<Luna::Point, f32>( args.m_Selection, &Point::GetPositionX, &Point::SetPositionX );
    args.m_Enumerator->AddValue<Luna::Point, f32>( args.m_Selection, &Point::GetPositionY, &Point::SetPositionY );
    args.m_Enumerator->AddValue<Luna::Point, f32>( args.m_Selection, &Point::GetPositionZ, &Point::SetPositionZ );
    args.m_Enumerator->Pop();
  }
  args.m_Enumerator->Pop();
}

bool Point::Pick( PickVisitor* pick )
{
  return pick->PickPoint( GetPosition() );
}

void Point::Evaluate( GraphDirection direction )
{
  __super::Evaluate(direction);

  switch (direction)
  {
  case GraphDirections::Downstream:
    {
      m_ObjectBounds.minimum = GetPosition() - Math::Vector3 (1.f,1.f,1.f);
      m_ObjectBounds.maximum = GetPosition() + Math::Vector3 (1.f,1.f,1.f);
      break;
    }
  }
}

Math::Matrix4 PointTranslateManipulatorAdapter::GetFrame(ManipulatorSpace space)
{
  // base object manip frame
  Math::Matrix4 m = m_Point->GetTransform()->GetGlobalTransform();

  // if we are pivoting then just use the value
  m = Math::Matrix4 (GetValue()) * m;

  if (space == ManipulatorSpaces::Object)
  {
    return m;
  }
  else
  {
    Math::Matrix4 frame = Math::Matrix4::Identity;

    frame.t.x = m.t.x;
    frame.t.y = m.t.y;
    frame.t.z = m.t.z;

    return frame;
  }
}

Math::Matrix4 PointTranslateManipulatorAdapter::GetObjectMatrix()
{
  return GetNode()->GetTransform()->GetGlobalTransform();
}

Math::Matrix4 PointTranslateManipulatorAdapter::GetParentMatrix()
{
  return GetNode()->GetTransform()->GetGlobalTransform();
}
