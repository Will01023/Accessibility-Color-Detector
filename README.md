# Accessibility Color Detector

**[Download here](https://github.com/Will01023/Accessibility-Color-Detector/releases/latest/download/AccessibilityColorDetector.exe)** - no install needed, just run it.

A tiny Windows app that tells you what color is under your mouse cursor. That's it.

Hover over anything on your screen and a small overlay follows your cursor showing the color name - simple names like "Red", "Blue", "Cyan", "Pink", not "Cornflower Blue #6495ED".

## Why

If you're colorblind (or just bad at telling teal from cyan), this saves you from guessing. No menus, no settings, no setup. Run it and go.

## How it works

Reads the pixel under your cursor using the Windows GDI API (`GetPixel`), converts the RGB value to HSL, and maps it to a plain English color name. A small floating overlay shows the swatch and the name.

The color names are intentionally broad: Red, Orange, Yellow, Lime, Green, Cyan, Blue, Purple, Pink, Black, White, Gray, Dark Gray, Light Gray. Nothing fancy.

## Usage

Run `AccessibilityColorDetector.exe`. A small overlay appears near your cursor.

- Drag the overlay to reposition it
- Close it from the taskbar like any other app
- Or hit Ctrl+Shift+Q to quit

## Building from source

Needs Visual Studio 2022 with the C++ workload and Windows SDK.

```
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The exe lands in `build/Release/`.

## Dependencies

None beyond what ships with Windows. Only links against `dwmapi.lib` which is part of the Windows SDK. The screen reading uses GDI which is built into every copy of Windows.

## Limitations

- The color classifier is simple on purpose. It won't tell you "salmon" vs "coral", just "Pink" or "Orange"
- Some hardware-accelerated or DX-exclusive fullscreen windows may return black pixels through GDI
