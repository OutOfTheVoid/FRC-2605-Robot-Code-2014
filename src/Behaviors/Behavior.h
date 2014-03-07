#ifndef SHS_2605_BEHAVIOR_H
#define SHS_2605_BEHAVIOR_H

class Behavior
{
public:

	virtual ~Behavior () {};

	virtual void Start () = 0;
	virtual void Update () = 0;
	virtual void Stop () = 0;
	virtual void Restart () = 0;

};

#endif
