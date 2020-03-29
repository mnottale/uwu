# Ultralight with Unity

UwU is a bridge between Ultralight and Unity allowing you to use interfaces made
in html within Unity.

It supports loading urls and files, mouse and (basic) keyboard interaction,
executing javascript in the loaded page, and calling C# functions from javascript
(asynchronously).

# Installing

Download an ultralight SDK from their download page. Then download the UwU native
component from the releases tab, or build your own dll/so.

All the dll/so from ultralight must be copied to the Unity editor binary dir, as
well as the UwU dll/so. That UwU native component *must* be called `ulbridge.so`
on all platforms.

Finally copy the ultralight.cs file in your Assets/scripts directory.

# Using

Instantiate an ultralight script component on any GameObject.

If the "Is Gui" inspector setting is true, it will render as a GUI on screen.
Otherwise it will render using the `Sprite renderer` attached to the same
game object.

You can set the url to an `http` or `https` url to hit online resources, or
use a `file` url to get a local filesystem resource (the root is the
current working directory for now).

## Javascript

Execute Javascript by calling `ExecJavascript`. Note that due to the asynchronous
nature of the binding this cannot return the result of the evaluation.

To call C# code from Javascript, register a function with
`UltralightManager.Instance().RegisterCallback("myFunctionName", callback);`, and
invoke it in javascript by doing `window.nativeCall("myFunctionName", "payload");`.
Payload must be a string.

# FAQ

### How to get a transparent background?

Don't set a background color. Instead use a div that covers the whole page with
an alpha component in it's color (`color = #ffffff70` for instance).

### It crashes, what should I do!

Ensure you do not call any of the synchronous functions within your code. Also
ensure that you don't make any call on a view *before* it is created.


# Design rationale.

The UwU native component runs in its own native thread that persists between
successive runs in the editor. What will *not* work:
- Running UwU in the main thread: the graphics commands Ultralight uses conflict
with unity.
- Running UwU in a C# thread: the thread will be killed when you exit play mode,
and recreating a new one on next play will crash Ultralight.
- Having a C# callback on UwU thread each frame. Fails because the C# runtime then
registers the thread, which prevents Unity from unloading the AppDomain when you
exit play mode.