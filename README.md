# FrameTimer
FrameTimer is a tiny header-only C++ library for Unreal Engine that provides a class for delaying the execution of arbitrary functions by any number of frames.

Just feed FrameTimer lambda expressions and the number of frames to delay function execution for, and it will take care of the rest!

## Installation
Clone the repository to your project's Public source directory:

```bash
cd [YOUR PROJECT SOURCE DIR]/Public/
git clone https://github.com/CYBERGEM-CORPORATION/FrameTimer
```

Or you might prefer to add it as a submodule (if your source dir is part of a git repository):

```bash
cd [YOUR PROJECT SOURCE DIR]/Public/
git submodule add https://github.com/CYBERGEM-CORPORATION/FrameTimer
```

Then just include it anywhere you want to use it:

```cpp
#include "FrameTimer/FrameTimer.h"
```

## Usage
First, create a FrameTimer instance somewhere convenient (perhaps as a member variable of another object):

```cpp
Cybergem::FFrameTimer FrameTimer;
```

By default, FrameTimer does not start ticking immediately after construction, in order to prevent it from ticking in things like CDOs, but FrameTimer needs to tick each frame in order to do its work, and there are two ways to go about it.

The first way is to call StartTicking() on FrameTimer once you want it to start doing work. It's best to do this somewhere like in an AActor's BeginPlay() rather than in its constructor (which could cause FrameTimer to tick in a CDO), so that it won't begin ticking until gameplay starts:

```cpp
void AFooActor::BeginPlay()
{
	Super::BeginPlay();

	// FrameTimer will now tick on its own
	FrameTimer.StartTicking();
}
```

The second way is to tick FrameTimer manually inside of the Tick() of another object. This can be useful if you need to ensure that the functions queued in FrameTimer execute in their respective frames before certain other logic is executed (since the Tick() order of objects is not always known or guaranteed):

```cpp
void AFooActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Manually tick FrameTimer
	FrameTimer.Tick(DeltaTime);
}
```

Then, simply feed FrameTimer the number of frames to delay function execution for, and a lambda expression:

```cpp
// This function will execute after 60 frames have elapsed, printing a debug message
FrameTimer.Create(60, []()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("60 frame delay!"));  
});
```

The safest way to call functions on UObject derived classes is to capture their pointers by value and create TWeakObjectPtrs, so that when the function is called in the future by FrameTimer it can first check to ensure that the pointers are still valid.

FrameTimer comes with a small helper utility function for creating TWeakObjectPtrs:

```cpp
// This function will execute after 100 frames have elapsed,
// and will check to make sure that the captured pointers still point to valid UObjects
FrameTimer.Create(100, [=]()
{
	auto SafeThis = FrameTimer.MakeWeakPtr(this);
	auto SafeOtherObjectPtr = FrameTimer.MakeWeakPtr(OtherObjectPtr);

	if (SafeThis.IsValid() && SafeOtherObjectPtr.IsValid())
	{
		// Do something with SafeThis and SafeOtherObjectPtr
	}
});
```

## Warning!
FrameTimer is experimental software, use at your own risk! 😊

## License
Published under the [MIT](https://choosealicense.com/licenses/mit/) license.