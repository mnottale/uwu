using UnityEngine;
using System;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections;
using System.Collections.Generic;



public class ULBridge
{
    public delegate void Callback();
    public delegate void JSNativeCall([MarshalAs(UnmanagedType.LPStr)]string name, [MarshalAs(UnmanagedType.LPStr)]string value);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_init();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_shutdown();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_start_thread();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_set_callback(Callback cb);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_render();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_update();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_view_create       ([MarshalAs(UnmanagedType.LPStr)] string name, int w, int h);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool ulbridge_view_is_dirty     ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ulbridge_view_get_pixels ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_view_unlock_pixels([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_view_load_html    ([MarshalAs(UnmanagedType.LPStr)] string name, [MarshalAs(UnmanagedType.LPStr)] string html);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_view_load_url     ([MarshalAs(UnmanagedType.LPStr)] string name, [MarshalAs(UnmanagedType.LPStr)] string url);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_view_resize       ([MarshalAs(UnmanagedType.LPStr)] string name, int w, int h);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ulbridge_view_width         ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ulbridge_view_height        ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ulbridge_view_stride        ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_create ([MarshalAs(UnmanagedType.LPStr)] string name, int w, int h);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_resize ([MarshalAs(UnmanagedType.LPStr)] string name, int w, int h);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_load_html([MarshalAs(UnmanagedType.LPStr)] string name, [MarshalAs(UnmanagedType.LPStr)] string html);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_load_url ([MarshalAs(UnmanagedType.LPStr)] string name, [MarshalAs(UnmanagedType.LPStr)] string url);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool ulbridge_async_view_is_dirty ([MarshalAs(UnmanagedType.LPStr)] string name);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ulbridge_async_view_get_pixels ([MarshalAs(UnmanagedType.LPStr)] string name, out int w, out int h, out int stride);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_delete_all_views();
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_mouse_event ([MarshalAs(UnmanagedType.LPStr)] string name, int x, int y, int type, int button);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_scroll_event ([MarshalAs(UnmanagedType.LPStr)] string name, int x, int y, int type);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_send_commands(JSNativeCall cb);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_eval_script  ([MarshalAs(UnmanagedType.LPStr)] string name, [MarshalAs(UnmanagedType.LPStr)] string script);
    [DllImport("ulbridge.so", CallingConvention = CallingConvention.Cdecl)]
    public static extern void ulbridge_async_view_key_event    ([MarshalAs(UnmanagedType.LPStr)] string name, int type, int vcode, int mods);
}

public class UltralightManager
{
    public delegate void JavascriptCallback(string payload);
    public class ViewData
    {
        public int viewWidth;
        public int viewHeight;
        public int w;
        public int h;
        public Color32[] data;
        public bool changed;
    }
    static private UltralightManager instance;
    private Dictionary<string, ViewData> views = new Dictionary<string, ViewData>();
    private Dictionary<string, JavascriptCallback> callbacks = new Dictionary<string, JavascriptCallback>();
    private int lastUpdateFrame;
    static public UltralightManager Instance()
    {
        if (instance == null)
            instance = new UltralightManager();
        return instance;
    }
    public void Detach()
    {
    }
    private bool mustStop = false;
    private UltralightManager()
    {
        Debug.Log("****INIT()");
        //ULBridge.ulbridge_set_callback(callback);
        ULBridge.ulbridge_start_thread();
        RegisterCallback("log",  val => Debug.Log("JS: " + val));
    }
    public void RegisterCallback(string name, JavascriptCallback callback)
    {
        callbacks.Add(name, callback);
    }
    public void ProcessCallback(string name, string value)
    {
        JavascriptCallback cb;
        if (callbacks.TryGetValue(name, out cb))
            cb(value);
        else
            Debug.Log("Received JS message to unknown target: " + name);
    }
    public void Update()
    {
        if (Time.frameCount == lastUpdateFrame)
            return;
        lastUpdateFrame = Time.frameCount;
        ULBridge.ulbridge_send_commands(this.ProcessCallback);
    }

    public void CreateView(string name, int w, int h)
    {
        ViewData v;
        if (views.TryGetValue(name, out v))
            throw new Exception("View already exists");
        ULBridge.ulbridge_async_view_create(name, w, h);
        //ULBridge.ulbridge_async_view_load_html(name, "<body> <h1>loading...</h1></body>");
        views.Add(name, new ViewData()
            {
                viewWidth = w,
                viewHeight = h,
                data = new Color32[w*h],
            });
    }
    public ViewData GetView(string name)
    {
        ViewData res = null;
        if (!views.TryGetValue(name, out res))
            return null;
        if (!ULBridge.ulbridge_async_view_is_dirty(name))
        {
            res.changed = false;
            return res;
        }
        // The texture can have a different size than the one requested,
        // but it will be black(unfilled) outside our requested size.
        int nw;
        int nh;
        int stride;
        var pixels = ULBridge.ulbridge_async_view_get_pixels(name, out nw, out nh, out stride);
        unsafe {
            if (pixels.ToPointer() == null)
                return null;
        }
        res.w = res.viewWidth;
        res.h = res.viewHeight;
        unsafe {
            var scolors = (Color32*)pixels.ToPointer();
            for (int y=0; y<res.viewHeight; ++y)
            {
                var start = scolors + y*stride/4;
                for (int x=0; x<res.viewWidth; ++x)
                    res.data[x+(res.viewHeight-y-1)*res.viewWidth] = start[x];
            }
        }
        res.changed = true;
        return res;
    }
}


public enum Anchor
{
    AnchorBegin,
    AnchorCenter,
    AnchorEnd
}

public class Ultralight: MonoBehaviour
{
    public int width = 640;
    public int height = 480;
    public int offsetX = 0;
    public int offsetY = 0;
    public bool isGui = true;
    public Anchor anchorX = Anchor.AnchorBegin;
    public Anchor anchorY = Anchor.AnchorBegin;
    public string url = "http://www.google.com";
    public bool loadUrlNow = false;
    public string javascript = "";
    public bool execJavascriptNow = false;
    bool created = false;
    Texture2D tex;
    int x;
    int y;
    public void Awake()
    {
    }
    public void OnDestroy()
    {
    }
    public void LoadURL(string url)
    {
        ULBridge.ulbridge_async_view_load_url(gameObject.name, url);
    }
    public void ExecJavascript(string js)
    {
        if (created)
            ULBridge.ulbridge_async_view_eval_script(gameObject.name, js);
        else
        {
            javascript = js;
            execJavascriptNow = true;
        }
    }
    void Update()
    {
        UltralightManager.Instance().Update();
        if (!created)
        {
            created = true;
            UltralightManager.Instance().CreateView(gameObject.name, width, height);
            ULBridge.ulbridge_async_view_load_url(gameObject.name, url);
        }
        if (loadUrlNow)
        {
            loadUrlNow = false;
            ULBridge.ulbridge_async_view_load_url(gameObject.name, url);
        }
        if (execJavascriptNow)
        {
            execJavascriptNow = false;
            ULBridge.ulbridge_async_view_eval_script(gameObject.name, javascript);
        }
        if (!isGui)
        {
            var vd = UltralightManager.Instance().GetView(gameObject.name);
            if (vd != null && vd.data != null && vd.changed)
            {
                vd.changed = false;
                if (tex == null || tex.width != vd.w || tex.height != vd.h)
                    tex = new Texture2D(vd.w, vd.h);
                tex.SetPixels32(vd.data);
                tex.Apply();
                SpriteRenderer sr = GetComponent<SpriteRenderer>();
                sr.sprite = Sprite.Create(tex, new Rect(0, 0, tex.width, tex.height),
                    new Vector2(0, 0), 70.0f);
            }
        }
        if (tex != null && isGui)
        {
            var mp = Input.mousePosition;
            mp.y = Screen.height - mp.y;
            var isIn = (mp.x >= x && mp.y >= y && mp.x <= x+tex.width && mp.y <= y+tex.height);
            if (isIn)
            {
                var emited = false;
                for (int b=0; b<3; ++b)
                {
                    if (Input.GetMouseButtonDown(b))
                    {
                        Debug.Log("button down");
                        ULBridge.ulbridge_async_view_mouse_event(gameObject.name, (int)mp.x-x, (int)mp.y-y, 1, b+1);
                        emited = true;
                    }
                    if (Input.GetMouseButtonUp(b))
                    {
                        Debug.Log("button up");
                        emited = true;
                        ULBridge.ulbridge_async_view_mouse_event(gameObject.name, (int)mp.x-x, (int)mp.y-y, 2, b+1);
                    }
                }
                if (!emited)
                    ULBridge.ulbridge_async_view_mouse_event(gameObject.name, (int)mp.x - x, (int)mp.y-y, 0, 0);
                if (Input.mouseScrollDelta.y != 0)
                    ULBridge.ulbridge_async_view_scroll_event(gameObject.name, 0, (int)Input.mouseScrollDelta.y, 1);
            }
        }
    }
    static private Dictionary<KeyCode, int> keycodeMap = new Dictionary<KeyCode, int>
    {
        {KeyCode.Backspace, 0x08},
        {KeyCode.Delete, 0x2E},
        {KeyCode.Tab, 0x09},
        {KeyCode.Clear, 0x0C},
        {KeyCode.Return, 0x0D},
        {KeyCode.Pause, 0x13},
        {KeyCode.Escape, 0x1B},
        {KeyCode.Space, 0x20},
        {KeyCode.UpArrow, 0x26},
        {KeyCode.DownArrow, 0x28},
        {KeyCode.LeftArrow, 0x25},
        {KeyCode.RightArrow, 0x27},
        {KeyCode.Insert, 0x2D},
        {KeyCode.Home, 0x24},
        {KeyCode.End, 0x23},
        {KeyCode.PageUp, 0x21},
        {KeyCode.PageDown, 0x22},
        {KeyCode.Keypad0, 0x60},
        {KeyCode.Keypad1, 0x61},
        {KeyCode.Keypad2, 0x62},
        {KeyCode.Keypad3, 0x63},
        {KeyCode.Keypad4, 0x64},
        {KeyCode.Keypad5, 0x65},
        {KeyCode.Keypad6, 0x66},
        {KeyCode.Keypad7, 0x67},
        {KeyCode.Keypad8, 0x68},
        {KeyCode.Keypad9, 0x69},
        {KeyCode.KeypadMultiply, 0x6A},
        {KeyCode.KeypadDivide, 0x6F},
        {KeyCode.KeypadPlus, 0x6B},
        {KeyCode.KeypadMinus, 0x6D},
        {KeyCode.F1, 0x70},
        {KeyCode.F2, 0x71},
        {KeyCode.F3, 0x72},
        {KeyCode.F4, 0x73},
        {KeyCode.F5, 0x74},
        {KeyCode.F6, 0x75},
        {KeyCode.F7, 0x76},
        {KeyCode.F8, 0x77},
        {KeyCode.F9, 0x78},
        {KeyCode.F10, 0x79},
        {KeyCode.F11, 0x7A},
        {KeyCode.F12, 0x7B},
        {KeyCode.F13, 0x7C},
        {KeyCode.F14, 0x7D},
        {KeyCode.F15, 0x7E},
        {KeyCode.Alpha0, 0x30},
        {KeyCode.Alpha1, 0x31},
        {KeyCode.Alpha2, 0x32},
        {KeyCode.Alpha3, 0x33},
        {KeyCode.Alpha4, 0x34},
        {KeyCode.Alpha5, 0x35},
        {KeyCode.Alpha6, 0x36},
        {KeyCode.Alpha7, 0x37},
        {KeyCode.Alpha8, 0x38},
        {KeyCode.Alpha9, 0x39},
        {KeyCode.DoubleQuote, 0xDE},
        {KeyCode.Quote, 0xDE},
        {KeyCode.Period, 0xBE},
        {KeyCode.Comma, 0xBC},
        {KeyCode.Plus, 0xBB},
        {KeyCode.Minus, 0xBD},
        {KeyCode.Slash, 0xBF},
        {KeyCode.A, 0x41},
        {KeyCode.B, 0x42},
        {KeyCode.C, 0x43},
        {KeyCode.D, 0x44},
        {KeyCode.E, 0x45},
        {KeyCode.F, 0x46},
        {KeyCode.G, 0x47},
        {KeyCode.H, 0x48},
        {KeyCode.I, 0x49},
        {KeyCode.J, 0x4A},
        {KeyCode.K, 0x4B},
        {KeyCode.L, 0x4C},
        {KeyCode.M, 0x4D},
        {KeyCode.N, 0x4E},
        {KeyCode.O, 0x4F},
        {KeyCode.P, 0x50},
        {KeyCode.Q, 0x51},
        {KeyCode.R, 0x52},
        {KeyCode.S, 0x53},
        {KeyCode.T, 0x54},
        {KeyCode.U, 0x55},
        {KeyCode.V, 0x56},
        {KeyCode.W, 0x57},
        {KeyCode.X, 0x58},
        {KeyCode.Y, 0x59},
        {KeyCode.Z, 0x5A},
        //{KeyCode.LeftCurlyBracket, 0xDB},
        //{KeyCode.Pipe, 0xDC},
        //{KeyCode.RightCurlyBracket, 0xDD},
        //{KeyCode.Tilde, 0xC0},
        {KeyCode.Numlock, 0x90},
        {KeyCode.CapsLock, 0x14},
        {KeyCode.ScrollLock, 0x91},
        {KeyCode.LeftShift, 0xA0},
        {KeyCode.RightShift, 0xA1},
        {KeyCode.LeftControl, 0xA2},
        {KeyCode.RightControl, 0xA3},
        {KeyCode.LeftAlt, 0x12},
        {KeyCode.RightAlt, 0x12},
        {KeyCode.Print, 0x2A},
    };
    void OnGUI()
    {
        if (!isGui || !created)
            return;
        var vd = UltralightManager.Instance().GetView(gameObject.name);
        if (vd == null || vd.data == null || !vd.changed)
            return;

        vd.changed = false;
        if (tex == null || tex.width != vd.w || tex.height != vd.h)
            tex = new Texture2D(vd.w, vd.h);
        tex.SetPixels32(vd.data);
        tex.Apply();

        x = 0;
        y = 0;
        if (anchorX == Anchor.AnchorCenter)
            x = Screen.width / 2 - tex.width / 2;
        if (anchorX == Anchor.AnchorEnd)
            x = Screen.width - tex.width;
        if (anchorY == Anchor.AnchorCenter)
            y = Screen.height / 2 - tex.height / 2;
        if (anchorY == Anchor.AnchorEnd)
            y = Screen.height - tex.height;
        x += offsetX;
        y += offsetY;
        GUI.DrawTexture(new Rect(x, y, width, height), tex, ScaleMode.StretchToFill, true);
    
        var mp = Input.mousePosition;
        var isIn = (mp.x >= x && mp.y >= y && mp.x <= x+tex.width && mp.y <= y+tex.height);
        if (isIn)
        {
            var ev = Event.current;
            if (ev.type == EventType.KeyDown || ev.type == EventType.KeyUp)
            {
                Debug.Log("key event");
                int vcode = 0;
                if (keycodeMap.TryGetValue(ev.keyCode, out vcode))
                { // we can map this
                    var mods = 0;
                    if (ev.shift)
                        mods |= (1 << 3);
                    if (ev.alt)
                        mods |= (1 << 0);
                    if (ev.control)
                        mods |= (1 << 1);
                    if (ev.command)
                        mods |= (1 << 2);
                    ULBridge.ulbridge_async_view_key_event(gameObject.name,
                        ev.type == EventType.KeyDown ? 1 : 0,
                        vcode, mods);
                }
            }
        }
    }
}