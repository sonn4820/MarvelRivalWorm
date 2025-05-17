#include "Game/Controller.hpp"

class PlayerController : public Controller
{
public:
	PlayerController(Game* owner, Character* character, char ID, char teamID);
	virtual ~PlayerController() = default;

	void Update(float deltaSeconds) override;
	bool IsPlayer() const override;

};