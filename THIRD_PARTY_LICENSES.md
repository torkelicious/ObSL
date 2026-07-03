# Third-Party Licenses

ObSL is built with the the following third-party software used under the terms of their own license, reproduced below.

---

## nlohmann/json

- **Project**: https://github.com/nlohmann/json
- **Version used**: v3.12.0 (see `CMakeLists.txt`)
- **Used for**: JSON-formatted output in `--lint` mode (`src/ScriptEntry.cpp`)
- **License**: MIT

```
MIT License

Copyright (c) 2013-2025 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### Components bundled within nlohmann/json

The `json.hpp` header itself incorporates a small number of third-party components under their own licenses, per [nlohmann/json's README](https://github.com/nlohmann/json#license)
