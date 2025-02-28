#pragma once
class SpikyMaterial : public Material<SpikyMaterial>
{
public:
	SpikyMaterial();
	~SpikyMaterial() override = default;

	SpikyMaterial(const SpikyMaterial& other) = delete;
	SpikyMaterial(SpikyMaterial&& other) noexcept = delete;
	SpikyMaterial& operator=(const SpikyMaterial& other) = delete;
	SpikyMaterial& operator=(SpikyMaterial&& other) noexcept = delete;

	//void SetDiffuseTexture(const std::wstring& assetFile);

protected:
	void InitializeEffectVariables() override;

private:
	float m_SpikeLength{0.0f};
};

