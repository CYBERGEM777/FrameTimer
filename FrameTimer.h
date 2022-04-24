// COPYRIGHT 2021 CYBERGEM CORPORATION [WWW.CYBERGEM.NET]

#pragma once

#include "CoreMinimal.h"
#include <functional>
#include "Containers/Ticker.h"

//
//	UNREAL FRAME TIMER
//

namespace Cybergem {

//
//	Convert lambda expressions to std::function.
//
//	This template witchery was found in an answer here:
//	https://stackoverflow.com/questions/23594969/operator-overloading-for-lambdas
//

template<typename T>
struct PayloadFuncType
{
	using Type = void;
};


template<typename Ret, typename Class, typename... Args>
struct PayloadFuncType<Ret(Class::*)(Args...) const>
{
	using Type = std::function<Ret(Args...)>;
};


// Create std::function from lambda
template<typename F>
typename PayloadFuncType<decltype(&F::operator())>::Type LambdaToFunction(F const& Func)
{
	return Func;
}


/**
 * Abstract FramePayload class.
 */
class FFramePayload_Abstract
{
public:

	virtual ~FFramePayload_Abstract() {}

	FORCEINLINE virtual bool TickFrame() = 0;

	FORCEINLINE bool IsDone() const { return bDone; }

protected:

	bool bDone = false;
};
	

/**
 * A class that executes a function after being ticked for an arbitrary number of frames.
 */
template <typename... Args> 
class FFramePayload : public FFramePayload_Abstract
{
public:

	FFramePayload() = delete;

	FFramePayload(uint64 _FrameDelay, std::function<void(Args...)> _Func)
	: CreationFrame(GFrameCounter)
	, FrameDelay(_FrameDelay > 0 ? _FrameDelay : 1) // Minimum FrameDelay is 1
	, Func(_Func)
	{}

	// Returns true once the frames have elapsed and the function has been executed.
	FORCEINLINE virtual bool TickFrame() override
	{
		// Ensure that at least one frame has elapsed.
		//
		// This prevents a 1 frame delay from executing the same frame it was
		// created in the event that it was queued in the Tick() of an object
		// that ticked before the FrameTimer did in the same frame.
		if (!bDone && GFrameCounter > CreationFrame && --FrameDelay == 0)
		{
			bDone = true;
			Func();
		}

		return bDone;
	}

private:

	// The frame number in which this payload was created
	uint64 CreationFrame;
	// The number of frames to wait before calling Func.
	// Func will be executed on frame CreationFrame + FrameDelay.
	uint64 FrameDelay;
	// The function to execute
	std::function<void(Args...)> Func;
};


/**
 * A class that executes arbitrary functions after a delay measured in frames.
 */
class FFrameTimer
{
public:

	FFrameTimer() : bTickRegistered(false) {}

	~FFrameTimer()
	{
		if (bTickRegistered)
		{
			FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
		}
	}

	void StartTicking()
	{
		if (!bTickRegistered)
		{
			bTickRegistered = true;
			TickDelegate = FTickerDelegate::CreateRaw(this, &FFrameTimer::Tick);
			TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);
		}
	}

	// Must return true in order to continue being ticked by CoreTicker
	bool Tick(float DeltaTime)
	{
		// Don't tick if there's no work to do, or if already ticked this frame
		if (AllTimersDone() || LastTickFrame == GFrameCounter)
		{
			return true;
		}

		LastTickFrame = GFrameCounter;
		TArray<int32> DeadPayloads;

		// Not iterating over array backwards here so that the order
		// of payload execution matches the order they were created.
		for (int32 i = 0; i < FramePayloads.Num(); ++i)
		{
			if (auto Payload = FramePayloads[i])
			{
				// Returns true if payload deployed
				if (!Payload->TickFrame())
				{
					// Payload not deployed
					continue;
				}
			}

			DeadPayloads.Add(i);
		}

		for (int32 j = DeadPayloads.Num() - 1; j >= 0; --j)
		{
			FramePayloads.RemoveAt(DeadPayloads[j]);
		}

		return true;
	}

	// Func will be executed after FrameDelay frames have elapsed.
	template <typename F>
	void Create(uint64 FrameDelay, F const& Func)
	{
		// Execute function immediately if for some reason no delay was requested
		if (FrameDelay == 0)
		{
			Func();
		}
		else
		{
			Create_Internal(FrameDelay, LambdaToFunction(Func));
		}
	}

	// Returns the number of FramePayloads that have not yet elapsed.
	FORCEINLINE int32 Num() const { return FramePayloads.Num(); }

	// Returns true if all FramePayloads have elapsed and executed their functions.
	FORCEINLINE bool AllTimersDone() const { return FramePayloads.Num() == 0; }

	// Utility function for creating weak pointers.
	//
	// It's a good idea to capture pointers to UObject derived classes
	// as TWeakObjectPtrs in lambda functions, so you can be sure
	// the object is still valid once the function is called.
	template <typename T>
	FORCEINLINE static TWeakObjectPtr<T> MakeWeakPtr(T* t)
	{
		return TWeakObjectPtr<T>(t);
	}

protected:

	// Storing Shared Pointers of a base class so that if
	// different types of FramePayloads are added in the
	// future they can all be stored in the same array.
	TArray<TSharedPtr<FFramePayload_Abstract>> FramePayloads;

private:

	bool bTickRegistered;
	uint64 LastTickFrame;
	FTickerDelegate TickDelegate;
	FDelegateHandle TickDelegateHandle;

	template <typename... Args>
	void Create_Internal(uint64 FrameDelay, std::function<void(Args...)> Func)
	{
		FramePayloads.Emplace(
			TSharedPtr<FFramePayload_Abstract>(
				new FFramePayload<Args...>(FrameDelay, Func)
		));
	}
};

}; // namespace Cybergem

