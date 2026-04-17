#pragma once

class Imagination 
{
	
protected:
	std::string _debugName;

public:
	Imagination(const std::string& pName = "Imagination");
	virtual ~Imagination();

	virtual void StartDream() {}
	virtual void Dream(float pDeltaTime) {}
	virtual void EndDream() {};

	virtual void OnEvent(Event& pEvent) {}

	inline const std::string& GetName() const { return _debugName; }
	//virtual void ImGui
};