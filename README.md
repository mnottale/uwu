# Ultralight with Unity

UwU is a bridge between Ultralight and Unity allowing you to use interfaces made
in html within Unity.

It supports loading urls and files, mouse and (basic) keyboard interaction,
executing javascript in the loaded page, and calling C# functions from javascript.

# Installing

Download ultralight SDK 1.2.1 from their download page for your platform.
Then download the UwU native component from the releases tab, or build your own dll/so.

Building on linux (assuming ultralight SDK in ./uldsk:

```sh
g++ -fPIC -shared -o  libulbridge.so -Iulsdk/include ulbridge.cc -Lulsdk/bin -lUltralightCore -lUltralight -lWebCore -lAppCore '-Wl,-rpath,$ORIGIN'
```

Copy libulbridge.so (ulbridge.dll on windows) into Assets/Plugins directory (creating it if needed). Copy also all .so/.dll from ultralight sdk "bin" directory.


Finally copy the Ultralight.cs and UltralightManager.cs files in your
Assets/scripts directory.

# Using

Enable unsafe code in your unity project: go to project settings -> player -> allow unsafe code (all the way down).

Enable preloading for ulbridge.dll/libulbridge.so by clicking on the file in the asset browser, and ticking the "preload" button.

Instantiate UltralightManager once on any GameObject. This singleton is responsible
for calling ultralight render and update, as well as dispatching JS callbacks.

Instantiate an ultralight script component on any GameObject to make a view.

If the "Is Gui" inspector setting is true, it will render as a GUI on screen.
Otherwise it will render using the `Sprite renderer` attached to the same
game object.

You can set the url to an `http` or `https` url to hit online resources, or
use a `file` url to get a local filesystem resource (the root is the
current working directory for now).

## Javascript

Execute Javascript by calling `ExecJavascript`.

To call C# code from Javascript, register a function with
`UltralightManager.Instance().RegisterCallback("myFunctionName", callback);`, and
invoke it in javascript by doing `window.nativeCall("myFunctionName", "payload");`.
Payload must be a string.

# FAQ

### How to get a transparent background?

Don't set a background color. Instead use a div that covers the whole page with
an alpha component in it's color (`color = #ffffff70` for instance).
