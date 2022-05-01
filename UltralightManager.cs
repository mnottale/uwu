using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;

public class UltralightManager: MonoBehaviour
{
    public delegate void JavascriptCallback(string payload);
    private static UltralightManager instance;
    public static UltralightManager Instance() { return instance;}
    private Dictionary<string, JavascriptCallback> callbacks = new Dictionary<string, JavascriptCallback>();
    public void Start()
    {
        Debug.Log("****INIT()");
        instance = this;
        ULBridge.ulbridge_init(false);
        ULBridge.ulbridge_set_command_callback(this.ProcessCallback);
        RegisterCallback("log",  val => Debug.Log("JS: " + val));
    }
    public void RegisterCallback(string name, JavascriptCallback callback)
    {
        callbacks.Add(name, callback);
    }
    public void ProcessCallback(string name, string value)
    {
        Debug.Log($"Received JS callback: {name} : {value}");
        JavascriptCallback cb;
        if (instance.callbacks.TryGetValue(name, out cb))
            cb(value);
        else
            Debug.Log("Received JS message to unknown target: " + name);
    }
    public void Update()
    {
        ULBridge.ulbridge_update();
        ULBridge.ulbridge_render();
    }
}