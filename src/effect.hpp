class Actor;

class Effect
{
public:
	virtual bool applyTo(Actor *actor) = 0;
};

class HealthEffect : public Effect
{
public:
	float amount;
	const char *message;

	HealthEffect(float amount, const char *message);
	bool applyTo(Actor *actor);
};

