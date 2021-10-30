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

Then simply feed it the number of frames to delay function execution for, and a lambda expression:

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