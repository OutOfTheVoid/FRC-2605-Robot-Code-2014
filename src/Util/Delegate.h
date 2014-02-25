#ifndef SHS_2605_DELEGATE_H
#define SHS_2605_DELEGATE_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

template <class Return>
class Delegate
{
	
public:
	
	virtual Return operator() () = 0;
	virtual Return Call () = 0;
	
	virtual ~Delegate () {};
	
};

template <class Return, class Arg1>
class Delegate2
{
	
public:
	
	virtual Return operator() ( Arg1 ) = 0;
	virtual Return Call ( Arg1 ) = 0;

	virtual ~Delegate2 () {};
	
};

template <class Return, class Arg1, class Arg2>
class Delegate3
{
	
public:
	
	virtual Return operator() ( Arg1, Arg2 ) = 0;
	virtual Return Call ( Arg1, Arg2 ) = 0;

	virtual ~Delegate3 () {};
	
};

template <class Return, class Arg1, class Arg2, class Arg3>
class Delegate4
{
	
public:
	
	virtual Return operator() ( Arg1, Arg2, Arg3 ) = 0;
	virtual Return Call ( Arg1, Arg2, Arg3 ) = 0;

	virtual ~Delegate4 () {};
	
};

template <class Return, class Arg1, class Arg2, class Arg3, class Arg4>
class Delegate5
{
	
public:
	
	virtual Return operator() ( Arg1, Arg2, Arg3, Arg4 ) = 0;
	virtual Return Call ( Arg1, Arg2, Arg3, Arg4 ) = 0;

	virtual ~Delegate5 () {};
	
};

template <class Return>
class StaticDelegate : public Delegate <Return>
{
public:
	
	StaticDelegate ( Return ( * Function ) ( void ) )
	{
		
		func = Function;
		
	};
	
	~StaticDelegate () {};
	
	Return Call ()
	{
		
		return ( * func ) ();
		
	};
	
	Return operator() ()
	{
		
		return ( * func ) ();
		
	};
	
private:
	
	Return ( * func ) ( void );
	
};

template <class Return, class Arg1>
class StaticDelegate2 : public Delegate2 <Return, Arg1>
{
public:
	
	StaticDelegate2 ( Return ( * Function ) ( Arg1 ) )
	{
		
		func = Function;
		
	};
	
	~StaticDelegate2 () {};
	
	Return Call ( Arg1 a )
	{
		
		return ( * func ) ( a );
		
	};
	
	Return operator() ( Arg1 a )
	{
		
		return ( * func ) ( a );
		
	};
	
private:
	
	Return ( * func ) ( Arg1 );
	
};

template <class Return, class Arg1, class Arg2>
class StaticDelegate3 : public Delegate3 <Return, Arg1, Arg2>
{
public:
	
	StaticDelegate3 ( Return ( * Function ) ( Arg1, Arg2 ) )
	{
		
		func = Function;
		
	};
	
	~StaticDelegate3 () {};
	
	Return Call ( Arg1 a, Arg2 b )
	{
		
		return ( * func ) ( a, b );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b )
	{
		
		return ( * func ) ( a, b );
		
	};
	
private:
	
	Return ( * func ) ( Arg1, Arg2 );
	
};

template <class Return, class Arg1, class Arg2, class Arg3>
class StaticDelegate4 : public Delegate4 <Return, Arg1, Arg2, Arg3>
{
public:
	
	StaticDelegate4 ( Return ( * Function ) ( Arg1, Arg2, Arg3 ) )
	{
		
		func = Function;
		
	};
	
	~StaticDelegate4 () {};
	
	Return Call ( Arg1 a, Arg2 b, Arg3 c )
	{
		
		return ( * func ) ( a, b, c );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b, Arg3 c )
	{
		
		return ( * func ) ( a, b, c );
		
	};
	
private:
	
	Return ( * func ) ( Arg1, Arg2, Arg3 );
	
};

template <class Return, class Arg1, class Arg2, class Arg3, class Arg4>
class StaticDelegate5 : public Delegate5 <Return, Arg1, Arg2, Arg3, Arg4>
{
public:
	
	StaticDelegate5 ( Return ( * Function ) ( Arg1, Arg2, Arg3, Arg4 ) )
	{
		
		func = Function;
		
	};
	
	~StaticDelegate5 () {};
	
	Return Call ( Arg1 a, Arg2 b, Arg3 c, Arg4 d )
	{
		
		return ( * func ) ( a, b, c, d );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b, Arg3 c, Arg4 d )
	{
		
		return ( * func ) ( a, b, c, d );
		
	};
	
private:
	
	Return ( * func ) ( Arg1, Arg2, Arg3, Arg4 );
	
};

template <class Type, class Return>
class ClassDelegate : public Delegate <Return>
{
public:
	
	ClassDelegate ( Type * CallObject, Return ( Type :: * Function ) ( void ) )
	{
		
		FuncCall = Function;
		Object = CallObject;
		
	};
	
	~ClassDelegate () {};
	
	Return Call ()
	{
		
		return ( Object ->* FuncCall ) ();
		
	};
	
	Return operator() ()
	{
		
		return ( Object ->* FuncCall ) ();
		
	};
	
	void SetObject ( Type * FunctionObject )
	{
		
		Object = FunctionObject;
		
	};
	
private:
	
	Return ( Type::* FuncCall )( void );
	Type * Object;
	
};

template <class Type, class Return, class Arg1>
class ClassDelegate2 : public Delegate2 <Return, Arg1>
{
public:
	
	ClassDelegate2 ( Type * CallObject, void ( Type :: * Function ) ( Arg1 ) )
	{
		
		FuncCall = Function;
		Object = CallObject;
		
	};
	
	~ClassDelegate2 () {};
	
	Return Call ( Arg1 a )
	{
		
		return ( Object ->* FuncCall ) ( a );
		
	};
	
	Return operator() ( Arg1 a )
	{
		
		return ( Object ->* FuncCall ) ( a );
		
	};
	
	void SetObject ( Type * FunctionObject )
	{
		
		Object = FunctionObject;
		
	};
	
private:
	
	Return ( Type::* FuncCall )( Arg1 );
	Type * Object;
	
};

template <class Type, class Return, class Arg1, class Arg2>
class ClassDelegate3 : public Delegate3 <Return, Arg1, Arg2>
{
public:
	
	ClassDelegate3 ( Type * CallObject, void ( Type :: * Function ) ( Arg1, Arg2 ) )
	{
		
		FuncCall = Function;
		Object = CallObject;
		
	};
	
	~ClassDelegate3 () {};
	
	Return Call ( Arg1 a, Arg2 b )
	{
		
		return ( Object ->* FuncCall ) ( a, b );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b )
	{
		
		return ( Object ->* FuncCall ) ( a, b );
		
	};
	
	void SetObject ( Type * FunctionObject )
	{
		
		Object = FunctionObject;
		
	};
	
private:
	
	Return ( Type::* FuncCall )( Arg1, Arg2 );
	Type * Object;
	
};

template <class Type, class Return, class Arg1, class Arg2, class Arg3>
class ClassDelegate4 : public Delegate4 <Return, Arg1, Arg2, Arg3>
{
public:
	
	ClassDelegate4 ( Type * CallObject, void ( Type :: * Function ) ( Arg1, Arg2, Arg3 ) )
	{
		
		FuncCall = Function;
		Object = CallObject;
		
	};
	
	~ClassDelegate4 () {};
	
	Return Call ( Arg1 a, Arg2 b, Arg3 c )
	{
		
		return ( Object ->* FuncCall ) ( a, b, c );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b, Arg3 c )
	{
		
		return ( Object ->* FuncCall ) ( a, b, c );
		
	};
	
	void SetObject ( Type * FunctionObject )
	{
		
		Object = FunctionObject;
		
	};
	
private:
	
	Return ( Type::* FuncCall )( Arg1, Arg2, Arg3 );
	Type * Object;
	
};

template <class Type, class Return, class Arg1, class Arg2, class Arg3, class Arg4>
class ClassDelegate5 : public Delegate5 <Return, Arg1, Arg2, Arg3, Arg4>
{
public:
	
	ClassDelegate5 ( Type * CallObject, void ( Type :: * Function ) ( Arg1, Arg2, Arg3, Arg4 ) )
	{
		
		FuncCall = Function;
		Object = CallObject;
		
	};
	
	~ClassDelegate5 () {};
	
	Return Call ( Arg1 a, Arg2 b, Arg3 c, Arg4 d )
	{
		
		return ( Object ->* FuncCall ) ( a, b, c, d );
		
	};
	
	Return operator() ( Arg1 a, Arg2 b, Arg3 c, Arg4 d )
	{
		
		return ( Object ->* FuncCall ) ( a, b, c, d );
		
	};
	
	void SetObject ( Type * FunctionObject )
	{
		
		Object = FunctionObject;
		
	};
	
private:
	
	Return ( Type::* FuncCall )( Arg1, Arg2, Arg3, Arg4 );
	Type * Object;
	
};

#endif
