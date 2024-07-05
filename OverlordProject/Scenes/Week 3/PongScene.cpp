#include "stdafx.h"
#include "PongScene.h"


#include "Prefabs/SpherePrefab.h"
#include "Prefabs/CubePrefab.h"

void PongScene::Initialize()
{
	m_SceneContext.settings.showInfoOverlay = true;
	m_SceneContext.settings.drawPhysXDebug = false;
	m_SceneContext.settings.drawGrid = false;
	m_SceneContext.settings.enableOnGUI = true;
	m_SceneContext.settings.clearColor.x = 0;
	m_SceneContext.settings.clearColor.y = 0;
	m_SceneContext.settings.clearColor.z = 0;

	auto& physX = PxGetPhysics();
	auto pBouncyMaterial = physX.createMaterial(0.0f, 0.0f, 1.f);

	m_pCamera = new FixedCamera{};
	m_pCamera->GetTransform()->Translate(0, 35, 0);
	m_pCamera->GetTransform()->Rotate(90, 0, 0,true);

	AddChild(m_pCamera)->GetComponent<CameraComponent>()->SetActive();

	//ball
	m_pBall = new SpherePrefab(1, 20, XMFLOAT4{ Colors::Red });
	AddChild(m_pBall);

	
	auto pRigidBodyBall = m_pBall->AddComponent(new RigidBodyComponent(false));
	pRigidBodyBall->AddCollider(PxSphereGeometry{ 1.f }, *pBouncyMaterial);
	pRigidBodyBall->SetCollisionGroup(CollisionGroup::Group0);
	pRigidBodyBall->SetConstraint(RigidBodyConstraint::TransY, false);

	//peddles
	const XMFLOAT3 actorDimentionspeddle{ 1.f,2.f,5.f };
	//left
	m_pPeddleLeft = new CubePrefab(actorDimentionspeddle,XMFLOAT4{Colors::WhiteSmoke});
	AddChild(m_pPeddleLeft);
	m_pPeddleLeft->GetTransform()->Translate(-20.f, 0.f, 0.f);

	auto pRigidBodyPeddleLeft = m_pPeddleLeft->AddComponent(new RigidBodyComponent());
	pRigidBodyPeddleLeft->SetKinematic(true);
	pRigidBodyPeddleLeft->AddCollider(PxBoxGeometry(actorDimentionspeddle.x/2.f, actorDimentionspeddle.y / 2.f, actorDimentionspeddle.z / 2.f), *pBouncyMaterial);
	pRigidBodyBall->SetCollisionGroup(CollisionGroup::Group0);
	//pRigidBodyPeddleLeft->SetConstraint(RigidBodyConstraint::TransY, false);

	m_pPeddleRight = new CubePrefab(actorDimentionspeddle, XMFLOAT4{ Colors::WhiteSmoke });
	AddChild(m_pPeddleRight);
	m_pPeddleRight->GetTransform()->Translate(20.f, 0.f, 0.f);

	auto pRigidBodyPeddleRight = m_pPeddleRight->AddComponent(new RigidBodyComponent());
	pRigidBodyPeddleRight->SetKinematic(true);
	pRigidBodyPeddleRight->AddCollider(PxBoxGeometry(actorDimentionspeddle.x / 2.f, actorDimentionspeddle.y / 2.f, actorDimentionspeddle.z / 2.f), *pBouncyMaterial);
	pRigidBodyPeddleRight->SetCollisionGroup(CollisionGroup::Group0);
	//walls
	//triggers

	const XMFLOAT3 actorDimentionsWalls{ 1.f,2.f,30.f };
	const float wallsXPos{ 23.f };
	//left	
	m_pTriggerLeft = new GameObject();
	m_pTriggerLeft->GetTransform()->Translate(-wallsXPos,0.f,0.f);
	AddChild(m_pTriggerLeft);
	auto pRigidBodyTriggerLeft = m_pTriggerLeft->AddComponent(new RigidBodyComponent());
	pRigidBodyTriggerLeft->AddCollider(PxBoxGeometry{ actorDimentionsWalls.x / 2.f,actorDimentionsWalls.y / 2.f ,actorDimentionsWalls.z / 2.f }, *pBouncyMaterial,true);
	

	pRigidBodyTriggerLeft->SetConstraint(RigidBodyConstraint::TransX | RigidBodyConstraint::TransY | RigidBodyConstraint::TransZ, false);

	m_pTriggerLeft->SetOnTriggerCallBack([=](GameObject* , GameObject* , PxTriggerAction action) 
		{
			if (action == PxTriggerAction::ENTER)
			{
				ResetGame();
			}			
		});

	//right

	m_pTriggerRight = new GameObject();
	m_pTriggerRight->GetTransform()->Translate(wallsXPos, 0.f, 0.f);
	AddChild(m_pTriggerRight);
	auto pRigidBodyTriggerRight = m_pTriggerRight->AddComponent(new RigidBodyComponent());
	pRigidBodyTriggerRight->AddCollider(PxBoxGeometry{ actorDimentionsWalls.x / 2.f,actorDimentionsWalls.y / 2.f ,actorDimentionsWalls.z / 2.f }, *pBouncyMaterial, true);


	pRigidBodyTriggerRight->SetConstraint(RigidBodyConstraint::TransX | RigidBodyConstraint::TransY | RigidBodyConstraint::TransZ, false);

	m_pTriggerRight->SetOnTriggerCallBack([=](GameObject*, GameObject*, PxTriggerAction action)
		{
			if (action == PxTriggerAction::ENTER)
			{
				ResetGame();				
			}
		});
	//bottom
	const XMFLOAT3 actorDimentionsfloor{ 60.f,2.f,1.f };
	const float floorCeilingPos{ 15.f };

	const auto floorActor = physX.createRigidStatic(PxTransform{0,0,-floorCeilingPos });
	PxRigidActorExt::createExclusiveShape(*floorActor,
		PxBoxGeometry{ actorDimentionsfloor.x / 2.f,actorDimentionsfloor.y / 2.f ,actorDimentionsfloor.z / 2.f }, *pBouncyMaterial);
	
	GetPhysxProxy()->AddActor(*floorActor);

	//top
	const auto ceilingActor = physX.createRigidStatic(PxTransform{ 0,0,floorCeilingPos });
	PxRigidActorExt::createExclusiveShape(*ceilingActor,
		PxBoxGeometry{ actorDimentionsfloor.x / 2.f,actorDimentionsfloor.y / 2.f ,actorDimentionsfloor.z / 2.f }, *pBouncyMaterial);
	GetPhysxProxy()->AddActor(*ceilingActor);


	
	m_PeddleminZPos = (floorActor->getGlobalPose().p.z + (actorDimentionsfloor.z / 2.f) + (actorDimentionspeddle.z / 2.f));
	m_PeddlemaxZPos = (ceilingActor->getGlobalPose().p.z - (actorDimentionsfloor.z / 2.f) - (actorDimentionspeddle.z / 2.f));
}

void PongScene::Update()
{
#ifdef _DEBUG
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::pressed, 'C'))
	{
		Logger::LogInfo(L"Camera pos: x: {} y: {} z: {},\n camera rot x: {} ,y: {} ,z: {}",
			m_SceneContext.pCamera->GetTransform()->GetPosition().x,
			m_SceneContext.pCamera->GetTransform()->GetPosition().y,
			m_SceneContext.pCamera->GetTransform()->GetPosition().z,
			m_SceneContext.pCamera->GetTransform()->GetRotation().x,
			m_SceneContext.pCamera->GetTransform()->GetRotation().y,
			m_SceneContext.pCamera->GetTransform()->GetRotation().z);
	}
#endif

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::pressed, 'R'))
	{
		ResetGame();
	}

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::pressed, VK_SPACE) && !m_IsGameActive)
	{
		m_pBall->GetComponent<RigidBodyComponent>()->AddForce(XMFLOAT3{12.f,0,12.f},PxForceMode::eIMPULSE);
		m_IsGameActive = true;
	}

	const float speedPeddle{ 10.f };
	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, 'Q'))
	{
		float transform = m_pPeddleLeft->GetTransform()->GetPosition().z + (speedPeddle * m_SceneContext.pGameTime->GetElapsed());

		if (m_PeddlemaxZPos > transform)
		{
			m_pPeddleLeft->GetTransform()->GetPosition();
			m_pPeddleLeft->GetComponent<RigidBodyComponent>()->GetPxRigidActor()->is<PxRigidDynamic>()->setKinematicTarget(PxTransform{ -20.f,0,transform });
		}
		
		
	}

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, 'A'))
	{
		float transform = m_pPeddleLeft->GetTransform()->GetPosition().z + (-speedPeddle * m_SceneContext.pGameTime->GetElapsed());
		if (m_PeddleminZPos < transform)
		{
			m_pPeddleLeft->GetTransform()->GetPosition();
			m_pPeddleLeft->GetComponent<RigidBodyComponent>()->GetPxRigidActor()->is<PxRigidDynamic>()->setKinematicTarget(PxTransform{ -20.f,0,transform });
		}	
	}

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, VK_UP))
	{
		float transform = m_pPeddleRight->GetTransform()->GetPosition().z + (speedPeddle * m_SceneContext.pGameTime->GetElapsed());

		if (m_PeddlemaxZPos > transform)
		{
			m_pPeddleRight->GetTransform()->GetPosition();
			m_pPeddleRight->GetComponent<RigidBodyComponent>()->GetPxRigidActor()->is<PxRigidDynamic>()->setKinematicTarget(PxTransform{ 20.f,0,transform });
		}


	}

	if (m_SceneContext.pInput->IsKeyboardKey(InputState::down, VK_DOWN))
	{
		float transform = m_pPeddleRight->GetTransform()->GetPosition().z + (-speedPeddle * m_SceneContext.pGameTime->GetElapsed());
		if (m_PeddleminZPos < transform)
		{
			m_pPeddleRight->GetTransform()->GetPosition();
			m_pPeddleRight->GetComponent<RigidBodyComponent>()->GetPxRigidActor()->is<PxRigidDynamic>()->setKinematicTarget(PxTransform{ 20.f,0,transform });
		}
	}


	
}

void PongScene::Draw()
{
}

void PongScene::OnGUI()
{
	ImGui::Text("Start game: space\n Player 1: Q for up and A for down\n Player 2: Arrow_Up for up and Arrow_down for down\n\n");	
}

void PongScene::ResetGame()
{
	m_IsGameActive = false;
	m_pBall->GetTransform()->Translate(0, 0, 0);
	m_pBall->GetTransform()->Rotate(0, 0, 0);

	m_pPeddleLeft->GetTransform()->Translate(-20.f, 0.f, 0.f);
	m_pPeddleRight->GetTransform()->Translate(20.f, 0.f, 0.f);
}
