
#define _CRT_SECURE_NO_WARNINGS 1

#include <Ultralight/Ultralight.h>
#include <unordered_map>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif
#include <fcntl.h>

#ifdef ULBRIDGE_DEBUG
#define ULDEBUG(a) std::cerr << a << std::endl
#else
#define ULDEBUG(a) do {} while(false)
#endif

#ifdef _MSC_VER
#define ULBAPI __declspec(dllexport)
#else
#define ULBAPI
#endif

using namespace ultralight;

RefPtr<Renderer> renderer;

std::string toUTF8(String16 const& s)
{
  std::string res;
  res.resize(s.length());
  for (int i=0; i<s.length(); ++i)
    res[i] = s.udata()[i];
  return res;
}
class LocalFileSystem: public FileSystem
{
private:
  std::string base = "./";

public:
  void SetBase(std::string b)
  {
    base = b;
  }
  std::filesystem::path toPath(String16 const& sfx)
  {
    auto res = std::filesystem::path(base) / toUTF8(sfx);
    std::cerr << "got path " << res << std::endl;
    return res;
  }
  virtual bool FileExists(const String16& path)
  {
    return std::filesystem::exists(toPath(path));
  }

  ///
  /// Delete file, return true on success.
  ///
  virtual bool DeleteFile_(const String16& path) { return false; }

  ///
  /// Delete empty directory, return true on success.
  ///
  virtual bool DeleteEmptyDirectory(const String16& path) { return false;}

  ///
  /// Move file, return true on success.
  ///
  virtual bool MoveFile_(const String16& old_path, const String16& new_path) { return false;}

  ///
  /// Get file size, store result in 'result'. Return true on success.
  ///
  virtual bool GetFileSize(const String16& path, int64_t& result)
  { 
    result = std::filesystem::file_size(toPath(path));
    return true;
  }

  ///
  /// Get file size of previously opened file, store result in 'result'. Return true on success.
  ///
  virtual bool GetFileSize(FileHandle handle, int64_t& result)
  {
    struct stat st;
    fstat(handle, &st);
    result = st.st_size;
    return true;
  }
  

  ///
  /// Get file mime type (eg "text/html"), store result in 'result'. Return true on success.
  ///
  virtual bool GetFileMimeType(const String16& path, String16& result)
  { // lol what?
    auto ext = toPath(path).extension();
    if (ext == ".htm" || ext == ".html")
      result = String16("text/html");
    if (ext == ".js")
      result = String16("text/javascript");
    if (ext == ".png")
      result = String16("image/png");
    if (ext == ".jpg")
      result = String16("image/jpeg");
    return true;
  }

  ///
  /// Get file last modification time, store result in 'result'. Return true on success.
  ///
  virtual bool GetFileModificationTime(const String16& path, time_t& result) { return false;}

  ///
  /// Get file creation time, store result in 'result'. Return true on success.
  ///
  virtual bool GetFileCreationTime(const String16& path, time_t& result) { return false;}

  ///
  /// Get path type (file or directory).
  ///
  virtual MetadataType GetMetadataType(const String16& path)
  {
    return kMetadataType_File;
  }

  ///
  /// Concatenate path with another path component. Return concatenated result.
  ///
  virtual String16 GetPathByAppendingComponent(const String16& path, const String16& component)
  {
    String16 res(path);
    res += String16("/");
    res += component;
    return res;
  }

  ///
  /// Create directory, return true on success.
  ///
  virtual bool CreateDirectory_(const String16& path) { return false;}

  ///
  /// Get home directory path.
  ///
  virtual String16 GetHomeDirectory()
  {
    return String16("/");
  }

  ///
  /// Get filename component from path.
  ///
  virtual String16 GetFilenameFromPath(const String16& path)
  {
    return toPath(path).filename().string().c_str();
  }

  ///
  /// Get directory name from path.
  ///
  virtual String16 GetDirectoryNameFromPath(const String16& path)
  {
    return String16(std::filesystem::path(toUTF8(path)).parent_path().string().c_str());
  }

  ///
  /// Get volume from path and store free space in 'result'. Return true on success.
  ///
  virtual bool GetVolumeFreeSpace(const String16& path, uint64_t& result) { return false;}

  ///
  /// Get volume from path and return its unique volume id.
  ///
  virtual int32_t GetVolumeId(const String16& path) { return 0;}

  ///
  /// Get file listing for directory path with optional filter, return vector of file paths.
  ///
  virtual Ref<String16Vector> ListDirectory(const String16& path, const String16& filter)
  {
    return String16Vector::Create();
  }

  ///
  /// Open a temporary file with suggested prefix, store handle in 'handle'. Return path of temporary file.
  ///
  virtual String16 OpenTemporaryFile(const String16& prefix, FileHandle& handle)
  {
    return String16();
  }

  ///
  /// Open file path for reading or writing. Return file handle on success, or invalidFileHandle on failure.
  ///
  virtual FileHandle OpenFile(const String16& path, bool open_for_writing)
  {
#ifdef _MSC_VER
          return _open(toPath(path).string().c_str(), _O_RDONLY);
#else
          return open(toPath(path).string().c_str(), O_RDONLY);
#endif
  }

  ///
  /// Close previously-opened file.
  ///
  virtual void CloseFile(FileHandle& handle)
  {
#ifdef _MSC_VER
      _close(handle);
#else
      close(handle);
#endif
  }

  ///
  /// Seek currently-opened file, with offset relative to certain origin. Return new file offset.
  ///
  virtual int64_t SeekFile(FileHandle handle, int64_t offset, FileSeekOrigin origin)
  {
    int whence = 0;
    switch (origin)
    {
    case kFileSeekOrigin_Beginning:
      whence = SEEK_SET; break;
    case kFileSeekOrigin_Current:
      whence = SEEK_CUR; break;
    case kFileSeekOrigin_End:
      whence = SEEK_END; break;
    }
#ifdef _MSC_VER
    return _lseek(handle, offset, whence);
#else
    return lseek(handle, offset, whence);
#endif
  }

  ///
  /// Truncate currently-opened file with offset, return true on success.
  ///
  virtual bool TruncateFile(FileHandle handle, int64_t offset)
  {
    return false;
  }

  ///
  /// Write to currently-opened file, return number of bytes written or -1 on failure.
  ///
  virtual int64_t WriteToFile(FileHandle handle, const char* data, int64_t length) { return -1;}

  ///
  /// Read from currently-opened file, return number of bytes read or -1 on failure.
  ///
  virtual int64_t ReadFromFile(FileHandle handle, char* data, int64_t length)
  {
#ifdef _MSC_VER
      return _read(handle, data, length);
#else
    return read(handle, data, length);
#endif
  }

  ///
  /// Copy file from source to destination, return true on success.
  ///
  virtual bool CopyFile_(const String16& source_path, const String16& destination_path) { return false;}

};

class BridgeListener;

struct ViewData
{
  RefPtr<View> view;
  RefPtr<Bitmap> bitmap;
  bool dirty;
  int w;
  int h;
  int stride;
  std::vector<unsigned char> data;
  bool domReady = false;
  std::vector<std::string> pendingJS;
  std::unique_ptr<BridgeListener> listener;
};

static std::unordered_map<std::string, ViewData> views;
static std::mutex viewsLock;
static std::vector<std::pair<std::string, std::string>> commands;
static std::mutex commandsLock;

// FIXME: only ascii is supported basically
std::string toUTF8(JSStringRef& str)
{
  auto sz = JSStringGetLength(str);
  std::string res;
  res.resize(sz);
  auto cptr = JSStringGetCharactersPtr(str);
  for (int i=0; i<sz; ++i)
    res[i] = cptr[i];
  return res;
}
JSValueRef native_call(JSContextRef ctx, JSObjectRef function,
  JSObjectRef thisObject, size_t argumentCount,
  const JSValueRef arguments[], JSValueRef* exception)
{
  auto jName = JSValueToStringCopy(ctx, arguments[0], exception);
  auto jArg = JSValueToStringCopy(ctx, arguments[1], exception);
  std::string name = toUTF8(jName);
  std::string args = toUTF8(jArg);
  std::lock_guard g(commandsLock);
  commands.emplace_back(name, args);
  return JSValueMakeNull(ctx);
}

class BridgeListener: public LoadListener
{
public:
  BridgeListener(std::string name, View* view)
  : name(name)
  {
    view->set_load_listener(this);
  }
  virtual void OnDOMReady(View* view) override;
  std::string name;
};

void BridgeListener::OnDOMReady(View* view) {
  std::lock_guard lg(viewsLock);
  auto& v = views[name];
  v.domReady = true;
  for (auto const& js: v.pendingJS)
    view->EvaluateScript(String(js.c_str()));
  v.pendingJS.clear();
  // install native call handler
  JSContextRef ctx = view->js_context();
  JSStringRef name = JSStringCreateWithUTF8CString("nativeCall");
    JSObjectRef func = JSObjectMakeFunctionWithCallback(ctx, name, native_call);
    JSObjectRef globalObj = JSContextGetGlobalObject(ctx);
    JSObjectSetProperty(ctx, globalObj, name, func, 0, 0);
    JSStringRelease(name);
}



static bool initialized = false;
extern "C" ULBAPI void ulbridge_init() {
  std::cerr << "***ULBRIDGE INIT " << initialized << std::endl;
  if (initialized)
    return;
  initialized = true;
  // Do any custom config here
  Config config;
  //config.force_repaint = true;
  Platform::instance().set_config(config);
  //Platform::instance().set_gpu_driver(my_gpu_driver);
  Platform::instance().set_file_system(new LocalFileSystem());
  
  // Create the library
  renderer = Renderer::Create();
}

enum class RequestType
{
  CreateView,
  DeleteView,
  ResizeView,
  LoadHTML,
  LoadURL,
  DeleteAllViews,
  MouseEvent,
  ScrollEvent,
  EvalScript,
  KeyEvent,
};

struct Request
{
  Request() {};
  Request(RequestType type, std::string view, std::string sParam, int w, int h)
  {
    this->type = type;
    this->view = view;
    this->sParam = sParam;
    this->w = w;
    this->h = h;
  }
  static Request mouseEvent(std::string view, int x, int y, int type, int button)
  {
    Request r;
    r.view = view;
    r.type = RequestType::MouseEvent;
    r.w = x;
    r.h = y;
    r.eventType = type;
    r.button = button;
    return r;
  }
  static Request scrollEvent(std::string view, int x, int y, int type)
  {
    Request r;
    r.view = view;
    r.type = RequestType::ScrollEvent;
    r.w = x;
    r.h = y;
    r.eventType = type;
    return r;
  }
  static Request script(std::string view, std::string script)
  {
    Request r;
    r.view = view;
    r.type = RequestType::EvalScript;
    r.sParam = script;
    return r;
  }
  static Request key(std::string view, int type, int vcode, int mods)
  {
    Request r;
    r.view = view;
    r.type = RequestType::KeyEvent;
    r.w = type;
    r.h = vcode;
    r.button = mods;
    return r;
  }
  RequestType type;
  std::string view;
  std::string sParam;
  int w; // or type
  int h; // or code
  int eventType;
  int button; // or modifiers
};


extern "C" ULBAPI void ulbridge_render()
{
  renderer->Render();
}

extern "C" ULBAPI void ulbridge_update()
{
  renderer->Update();
}

extern "C" ULBAPI void ulbridge_view_create(const char* name, int w, int h)
{
  RefPtr<View> view = renderer->CreateView(w, h, true);
  views[name] = ViewData{ view, nullptr};
  views[name].listener = std::make_unique<BridgeListener>(name, view.get());
}

extern "C" ULBAPI bool ulbridge_view_is_dirty(const char* name)
{
  auto it = views.find(name);
  if (it == views.end())
    return false;
  else
    return it->second.view->is_bitmap_dirty();
}
extern "C" ULBAPI void* ulbridge_view_get_pixels(const char* name)
{
  auto it = views.find(name);
  if (it == views.end())
    return nullptr;
  auto& vd = it->second;
  auto bitmap = vd.view->bitmap();
  vd.bitmap = bitmap;
  void* pixels = vd.bitmap->LockPixels();
  return pixels;
}

extern "C" ULBAPI int ulbridge_view_width(const char* name)
{
  return views[name].bitmap->width();
}

extern "C" ULBAPI int ulbridge_view_height(const char* name)
{
  return views[name].bitmap->height();
}

extern "C" ULBAPI int ulbridge_view_stride(const char* name)
{
  return views[name].bitmap->row_bytes();
}

extern "C" ULBAPI void ulbridge_view_unlock_pixels(const char* name)
{
  auto& vd = views[name];
  vd.bitmap->UnlockPixels();
  vd.bitmap = nullptr;
}

extern "C" ULBAPI void ulbridge_view_load_html(const char* name, const char* html)
{
  views[name].view->LoadHTML(html);
}

extern "C" ULBAPI void ulbridge_view_load_url(const char* name, const char* url)
{
  views[name].view->LoadURL(url);
}

extern "C" ULBAPI void ulbridge_view_resize(const char* name, int w, int h)
{
  views[name].view->Resize(w, h);
}

extern "C" ULBAPI void ulbridge_view_mouse_event(const char* name, int x, int y, int type, int button)
{
  MouseEvent evt{(MouseEvent::Type)type, x, y, (MouseEvent::Button)button};
  views[name].view->FireMouseEvent(evt);
}

extern "C" ULBAPI void ulbridge_view_scroll_event(const char* name, int x, int y, int type)
{
  ScrollEvent evt{(ScrollEvent::Type)type, x, y};
  views[name].view->FireScrollEvent(evt);
}

extern "C" ULBAPI void ulbridge_view_eval_script(const char* name, const char* script)
{
  auto it = views.find(name);
  if (it == views.end())
  {
    std::cerr << "Dropping evalscript for " << name << " : view does not exists" << std::endl;
    return;
  }
  auto& v = it->second;
  if (!v.domReady)
    v.pendingJS.push_back(script);
  else
    v.view->EvaluateScript(String(script));
}

using namespace ultralight::KeyCodes;

std::unordered_map<int, char> charMap = {
  {GK_A, 'a'},
  {GK_B, 'b'},
  {GK_C, 'c'},
  {GK_D, 'd'},
  {GK_E, 'e'},
  {GK_F, 'f'},
  {GK_G, 'g'},
  {GK_H, 'h'},
  {GK_I, 'i'},
  {GK_J, 'j'},
  {GK_K, 'k'},
  {GK_L, 'l'},
  {GK_M, 'm'},
  {GK_N, 'n'},
  {GK_O, 'o'},
  {GK_P, 'p'},
  {GK_Q, 'q'},
  {GK_R, 'r'},
  {GK_S, 's'},
  {GK_T, 't'},
  {GK_U, 'u'},
  {GK_V, 'v'},
  {GK_W, 'w'},
  {GK_X, 'x'},
  {GK_Y, 'y'},
  {GK_Z, 'z'},
  {GK_0, '0'},
  {GK_1, '1'},
  {GK_2, '2'},
  {GK_3, '3'},
  {GK_4, '4'},
  {GK_5, '5'},
  {GK_6, '6'},
  {GK_7, '7'},
  {GK_8, '8'},
  {GK_9, '9'},
  {GK_SPACE, ' '},
  
};
extern "C" ULBAPI void ulbridge_view_key_event(const char* name, int type, int vcode, int mods)
{
  auto it = views.find(name);
  if (it == views.end())
    return;
  KeyEvent ke;
  ke.type = type ? KeyEvent::kType_KeyDown : KeyEvent::kType_KeyUp;
  ke.modifiers = mods;
  ke.virtual_key_code = vcode;
  ke.native_key_code = 0;
  GetKeyIdentifierFromVirtualKeyCode(ke.virtual_key_code, ke.key_identifier);
  it->second.view->FireKeyEvent(ke);
  if (type)
  {
    auto cit = charMap.find(vcode);
    if (cit != charMap.end())
    {
      KeyEvent ke;
      ke.type = KeyEvent::kType_Char;
      ke.modifiers = mods;
      ke.virtual_key_code = vcode;
      ke.native_key_code = 0;
      GetKeyIdentifierFromVirtualKeyCode(ke.virtual_key_code, ke.key_identifier);
      char txt[2] = { cit->second, 0};
      if (mods & (1 << 3))
        txt[0] = toupper(txt[0]);
      ke.text = String(txt);
      ke.unmodified_text = String(txt);
      it->second.view->FireKeyEvent(ke);
    }
  }
}

extern "C" ULBAPI void ulbridge_shutdown() {
  std::cerr << "***ULBSHUTDOWN" << std::endl;
  views.clear();
  //renderer = nullptr;
  //initialized = false;
}

typedef void (*Callback)();
typedef void (*CommandCallback)(const char*, const char*);
static std::thread thread;
static bool stopThread = false;
static bool threadRunning = false;
static Callback callback = nullptr;
static std::vector<Request> pending;
static std::mutex pendingLock;

extern "C" ULBAPI void ulbridge_set_callback(Callback cb)
{
  std::cerr << "***SETCB " << cb << std::endl;
  callback = cb;
}
extern "C" ULBAPI void ulbridge_send_commands(CommandCallback cb)
{
  std::vector<std::pair<std::string, std::string>> pending;
  {
    std::lock_guard g(commandsLock);
    std::swap(commands, pending);
  }
  for (auto const& c: pending)
    cb(c.first.c_str(), c.second.c_str());
}
static void ulbridge_loop()
{
  ulbridge_init();
  while (!stopThread)
  {
    ULDEBUG("renderer...");
    renderer->Update();
    renderer->Render();
    if (callback)
      callback();
    { // lock
    std::lock_guard lg(viewsLock);
    ULDEBUG("views..." << views.size());
    for (auto& v: views)
    {
      if (v.second.view->is_bitmap_dirty())
      {
        auto bitmap = v.second.view->bitmap();
        v.second.dirty = true;
        v.second.w = bitmap->width();
        v.second.h = bitmap->height();
        v.second.stride = bitmap->row_bytes();
        v.second.data.resize(v.second.stride * v.second.h);
        void* pixels = bitmap->LockPixels();
        memcpy(&v.second.data[0], pixels, v.second.data.size());
        bitmap->UnlockPixels();
      }
    }
    ULDEBUG("processing...");
    std::vector<Request> processing;
    {
      std::lock_guard g(pendingLock);
      std::swap(pending, processing);
    }
    for (auto const& r: processing)
    {
      switch (r.type)
      {
      case RequestType::CreateView:
        ULDEBUG("create  " << r.view << "....");
        ulbridge_view_create(r.view.c_str(), r.w, r.h);
        ULDEBUG("...create done");
        break;
      case RequestType::DeleteView:
        break;
      case RequestType::ResizeView:
        ULDEBUG("resize " << r.view);
        ulbridge_view_resize(r.view.c_str(), r.w, r.h);
        break;
      case RequestType::LoadHTML:
        ULDEBUG("loadhtml " << r.view);
        ulbridge_view_load_html(r.view.c_str(), r.sParam.c_str());
        break;
      case RequestType::LoadURL:
        ULDEBUG("loadurl " << r.view);
        ulbridge_view_load_url(r.view.c_str(), r.sParam.c_str());
        break;
      case RequestType::DeleteAllViews:
        views.clear();
        break;
      case RequestType::MouseEvent:
        ulbridge_view_mouse_event(r.view.c_str(), r.w, r.h, r.eventType, r.button);
        break;
      case RequestType::ScrollEvent:
        ulbridge_view_scroll_event(r.view.c_str(), r.w, r.h, r.eventType);
        break;
      case RequestType::EvalScript:
        ULDEBUG("eval " << r.view);
        ulbridge_view_eval_script(r.view.c_str(), r.sParam.c_str());
        break;
      case RequestType::KeyEvent:
        ULDEBUG("key " << r.view);
        ulbridge_view_key_event(r.view.c_str(), r.w, r.h, r.button);
        break;
      }
    }
    } // lock
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  threadRunning = false;
}
extern "C" ULBAPI void ulbridge_start_thread()
{
  if (threadRunning)
    return;
  threadRunning = true;
  thread = std::thread(ulbridge_loop);
}

extern "C" ULBAPI void ulbridge_async_view_create(const char* name, int w, int h)
{
  std::lock_guard g(pendingLock);
  pending.emplace_back(RequestType::CreateView, name, std::string(), w, h);
}

extern "C" ULBAPI void ulbridge_async_view_resize(const char* name, int w, int h)
{
  std::lock_guard g(pendingLock);
  pending.emplace_back(RequestType::ResizeView, name, std::string(), w, h);
}

extern "C" ULBAPI void ulbridge_async_view_load_html(const char* name, const char* str)
{
  std::lock_guard g(pendingLock);
  pending.emplace_back(RequestType::LoadHTML, name, str, 0, 0);
}

extern "C" ULBAPI void ulbridge_async_view_load_url(const char* name, const char* str)
{
  std::lock_guard g(pendingLock);
  pending.emplace_back(RequestType::LoadURL, name, str, 0, 0);
}

extern "C" ULBAPI bool ulbridge_async_view_is_dirty(const char* name)
{
  std::lock_guard lg(viewsLock);
  auto it = views.find(name);
  if (it == views.end())
    return false;
  else
    return it->second.dirty;
}

extern "C" ULBAPI void* ulbridge_async_view_get_pixels(const char* name, int* w, int* h, int* stride)
{
  std::lock_guard lg(viewsLock);
  auto it = views.find(name);
  if (it == views.end())
    return nullptr;
  auto& v = it->second;
  *w = v.w;
  *h = v.h;
  *stride = v.stride;
  return &v.data[0];
}

extern "C" ULBAPI void ulbridge_async_delete_all_views()
{
  std::lock_guard g(pendingLock);
  pending.emplace_back(RequestType::DeleteAllViews, std::string(), std::string(), 0, 0);
}

extern "C" ULBAPI void ulbridge_async_view_mouse_event(const char* name, int x, int y, int type, int button)
{
  std::lock_guard g(pendingLock);
  pending.push_back(Request::mouseEvent(name, x, y, type, button));
}

extern "C" ULBAPI void ulbridge_async_view_scroll_event(const char* name, int x, int y, int type)
{
  std::lock_guard g(pendingLock);
  pending.push_back(Request::scrollEvent(name, x, y, type));
}

extern "C" ULBAPI void ulbridge_async_view_eval_script(const char* name, const char* script)
{
  std::lock_guard g(pendingLock);
  pending.push_back(Request::script(name, script));
}

extern "C" ULBAPI void ulbridge_async_view_key_event(const char* name, int type, int vcode, int mods)
{
  std::lock_guard g(pendingLock);
  pending.push_back(Request::key(name, type, vcode, mods));
}
