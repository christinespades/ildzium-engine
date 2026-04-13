let httpCallbacks = new Map();
let nextId = 1;

mergeInto(LibraryManager.library, {

  js_get_clipboard: function(cb, user) {
    navigator.clipboard.readText()
      .then(text => {
        const len = lengthBytesUTF8(text) + 1;
        const buf = _malloc(len);
        stringToUTF8(text, buf, len);

        setTimeout(() => {
          const fn = httpCallbacks.get(cb);
          if (fn) fn(buf, user);
          _free(buf);
        }, 0);
      })
      .catch(() => {
        setTimeout(() => {
          const fn = httpCallbacks.get(cb);
          if (fn) fn(0, user);
        }, 0);
      });
  },

  js_http_get: function(url, packPtr) {
    const str = UTF8ToString(url);

    if (!Module.httpCallbacks) Module.httpCallbacks = new Map();
    if (Module.nextId === undefined) Module.nextId = 1;

    const id = Module.nextId++;
    Module.httpCallbacks.set(id, packPtr);

    fetch(str)
      .then(r => r.text())
      .then(text => {
        const len = lengthBytesUTF8(text) + 1;
        const buf = _malloc(len);
        stringToUTF8(text, buf, len);

        Module._on_http_result(buf, id);
      })
      .catch(() => {
        Module._on_http_result(0, id);
      });
  },

  js_dispatch_http_result: function(dataPtr, id) {
    const packPtr = Module.httpCallbacks.get(id);
    if (!packPtr) return;

    Module.httpCallbacks.delete(id);

    const cb = getValue(packPtr, 'i32');
    const user = getValue(packPtr + 4, 'i32');

    Module.ccall(
      'invoke_http_callback',
      null,
      ['number', 'number', 'number'],
      [cb, user, dataPtr]
    );
  }

});