#include "Precompile.h"
#include "SceneNodePanel.h"

using namespace Luna;
using namespace Nocturnal;

SceneNodePanel::SceneNodePanel(PropertiesGenerator* generator, const OS_SelectableDumbPtr& selection)
: m_Selection (selection)
{
  m_Interpreter = m_Generator = generator;
  m_Expanded = true;
  m_Text = TXT( "Scene Node" );
}

SceneNodePanel::~SceneNodePanel()
{
}

void SceneNodePanel::Create()
{
  m_Generator->PushContainer();
  {
    m_Generator->AddLabel( TXT( "ID" ) );
    Inspect::Value* textBox = m_Generator->AddValue<Luna::SceneNode, TUID>(m_Selection, &Luna::SceneNode::GetID, &Luna::SceneNode::SetID);
    textBox->SetReadOnly(true);
  }
  m_Generator->Pop();

  m_Generator->PushContainer();
  {
    m_Generator->AddLabel( TXT( "Name" ) );
    m_Generator->AddValue<Luna::SceneNode, tstring>( m_Selection, &Luna::SceneNode::GetName, &Luna::SceneNode::SetGivenName );
  }
  m_Generator->Pop();

  m_Generator->PushContainer();
  {
    m_Generator->AddLabel( TXT( "Auto Name" ) );
    m_Generator->AddCheckBox<Luna::SceneNode, bool>( m_Selection, &Luna::SceneNode::UseAutoName, &Luna::SceneNode::SetUseAutoName );
  }
  m_Generator->Pop();

  m_Generator->PushPanel( TXT( "Membership" ) );
  {
    m_Generator->PushContainer();
    m_Generator->AddList< Luna::SceneNode, tstring >( m_Selection, &Luna::SceneNode::GetMembership, &Luna::SceneNode::SetMembership );
    m_Generator->Pop();
  }
  m_Generator->Pop();

  __super::Create();
}