#include "stdafx.h"
#include "ModelAnimator.h"
#include "Base/Logger.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
	m_pMeshFilter{pMeshFilter}
{
	SetAnimation(0);
}

void ModelAnimator::Update(const SceneContext& sceneContext)
{
	TODO_W6_();

	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		//auto passedTicks = ...
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		auto passedTicks = sceneContext.pGameTime->GetElapsed() * m_CurrentClip.ticksPerSecond * m_AnimationSpeed;
		if (passedTicks > m_CurrentClip.duration) passedTicks = fmod(passedTicks, m_CurrentClip.duration);
		//2. 
		//IF m_Reversed is true
		//	Subtract passedTicks from m_TickCount
		//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
		//ELSE
		//	Add passedTicks to m_TickCount
		//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
		if (m_Reversed)
		{
			m_TickCount -= passedTicks;
			if (m_TickCount < 0) m_TickCount += m_CurrentClip.duration;
		}
		else
		{
			m_TickCount += passedTicks;
			if (m_TickCount > m_CurrentClip.duration) m_TickCount -= m_CurrentClip.duration;
		}

		//3.
		//Find the enclosing keys
		AnimationKey keyA, keyB;
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for (int i = 0; i < m_CurrentClip.keys.size(); ++i)
		{
			const auto& currentKey = m_CurrentClip.keys[i];
			if (currentKey.tick > m_TickCount)
			{
				keyB = currentKey;
				keyA = m_CurrentClip.keys[i - 1];
				break;
			}
		}

		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		//Clear the m_Transforms vector
		//FOR every boneTransform in a key (So for every bone)
		//	Retrieve the transform from keyA (transformA)
		//	auto transformA = ...
		// 	Retrieve the transform from keyB (transformB)
		//	auto transformB = ...
		//	Decompose both transforms
		//	Lerp between all the transformations (Position, Scale, Rotation)
		//	Compose a transformation matrix with the lerp-results
		//	Add the resulting matrix to the m_Transforms vector
		float range = keyB.tick - keyA.tick;
		float distanceFromA = m_TickCount - keyA.tick;
		const auto blendFactor = distanceFromA / range;

		for (int i = 0; i < m_pMeshFilter->m_BoneCount; ++i)
		{
			const auto& transformA = keyA.boneTransforms[i];
			const auto& transformB = keyB.boneTransforms[i];

			XMVECTOR scaleA{};
			XMVECTOR rotationA{};
			XMVECTOR translationA{};
			XMMatrixDecompose(&scaleA, &rotationA, &translationA, XMLoadFloat4x4(&transformA));

			XMVECTOR scaleB{};
			XMVECTOR rotationB{};
			XMVECTOR translationB{};
			XMMatrixDecompose(&scaleB, &rotationB, &translationB, XMLoadFloat4x4(&transformB));

			const auto scale = XMVectorLerp(scaleA, scaleB, blendFactor);
			const auto rotation = XMQuaternionSlerp(rotationA, rotationB, blendFactor);
			const auto translation = XMVectorLerp(translationA, translationB, blendFactor);

			const XMMATRIX newTransform{ XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotation)
				* XMMatrixTranslationFromVector(translation) };

			XMStoreFloat4x4(&m_Transforms[i], newTransform);
		}
	}
}

void ModelAnimator::SetAnimation(const std::wstring& clipName)
{
	// Set m_ClipSet to false
	m_ClipSet = false;

	// Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	for (auto& clip : m_pMeshFilter->GetAnimationClips())
	{
		if (clip.name == clipName)
		{
			// If found, call SetAnimation(Animation Clip) with the found clip
			SetAnimation(clip);
			return;
		}
	}

	// If not found, call Reset
	Reset();

	// Log a warning with an appropriate message	
	Logger::LogWarning(L"Animation clip not found: {}", clipName);
}

void ModelAnimator::SetAnimation(UINT clipNumber)
{
	// Set m_ClipSet to false
	m_ClipSet = false;

	// Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (clipNumber >= m_pMeshFilter->GetAnimationClips().size())
	{
		// If not, call Reset and log a warning message
		Reset();			
		Logger::LogWarning(L"Invalid animation clip number: {}" , clipNumber);
		return;
	}

	// If clipNumber is valid, retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
	const auto& clip = m_pMeshFilter->GetAnimationClips()[clipNumber];

	// Call SetAnimation(AnimationClip clip)
	SetAnimation(clip);
}

void ModelAnimator::SetAnimation(const AnimationClip& clip)
{
	// Set m_ClipSet to true
	m_ClipSet = true;

	// Set m_CurrentClip
	m_CurrentClip = clip;

	// Call Reset(false)
	Reset(false);
}

void ModelAnimator::Reset(bool pause)
{
	//TODO_W6_()
	//If pause is true, set m_IsPlaying to false

	//Set m_TickCount to zero
	//Set m_AnimationSpeed to 1.0f

	//If m_ClipSet is true
	//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
	//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
	//Else
	//	Create an IdentityMatrix 
	//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
	// If pause is true, set m_IsPlaying to false
	if (pause)m_IsPlaying = false;

	// Set m_TickCount to zero
	m_TickCount = 0;

	// Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.0f;

	if (m_ClipSet)
	{
		// If m_ClipSet is true, retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		const auto& boneTransforms = m_CurrentClip.keys[0].boneTransforms;

		// Refill the m_Transforms vector with the new BoneTransforms using vector::assign
		m_Transforms.assign(boneTransforms.begin(), boneTransforms.end());
	}
	else
	{
		// If m_ClipSet is false, create an IdentityMatrix
		const auto identity = DirectX::XMMatrixIdentity();
		XMFLOAT4X4 matrix;
		DirectX::XMStoreFloat4x4(&matrix, identity);
		// Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) using vector::assign
		m_Transforms.assign(m_CurrentClip.keys[0].boneTransforms.size(), matrix);
	}
}
