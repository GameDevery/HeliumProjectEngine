#include "JointOrdering.h"
#include "Exceptions.h"

#include <algorithm>

namespace Content
{

  REFLECT_DEFINE_CLASS( JointOrdering )

void JointOrdering::EnumerateClass( Reflect::Compositor<JointOrdering>& comp )
{
  Reflect::Field* fieldJointOrdering = comp.AddField( &JointOrdering::m_JointOrdering, "m_JointOrdering" );
}


  JointOrdering::JointOrdering( u32 numRequiredJoints )
    : SceneNode()
  {
    SetRequiredJointCount( numRequiredJoints );
  }
  void JointOrdering::SetRequiredJointCount( u32 requiredJointCount )
  {
    m_JointOrdering.resize( requiredJointCount );
  }

  u32 JointOrdering::GetRequiredJointCount()
  {
    return (u32)m_JointOrdering.size();
  }

  void JointOrdering::AddJoint( const UniqueID::TUID& jointId )
  {
    m_JointOrdering.push_back( jointId );
  }

  void JointOrdering::Clear()
  {
    m_JointOrdering.clear();
  }

  bool JointOrdering::IsRequired( const UniqueID::TUID& jointId )
  {
    return std::find( m_JointOrdering.begin(), m_JointOrdering.end(), jointId ) != m_JointOrdering.end();
  }

  UniqueID::TUID JointOrdering::GetMasterJoint( const UniqueID::TUID& localJoint )
  {
    UniqueID::HM_TUID::iterator jointIt = m_LocalToMasterMap.find( localJoint );

    if ( jointIt == m_LocalToMasterMap.end() )
    {
      std::string localJointGuid;
      localJoint.ToString( localJointGuid );
      throw MissingJointException( localJointGuid, "could not find master joint for this local joint." );
    }  

    return jointIt->second;
  }

  UniqueID::TUID JointOrdering::GetLocalJoint( const UniqueID::TUID& masterJoint )
  {
    UniqueID::HM_TUID::iterator jointIt = m_MasterToLocalMap.find( masterJoint );

    if ( jointIt == m_MasterToLocalMap.end() )
    {
      std::string masterJointGuid;
      masterJoint.ToString( masterJointGuid );
      throw MissingJointException( masterJointGuid, "could not find local joint for this master joint." );
    }  

    return jointIt->second;
  }
}