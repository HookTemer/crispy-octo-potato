/**
 * @file src/platform/linux/input/inputtino_common.h
 * @brief Declarations for inputtino common input handling.
 */
#pragma once

// lib includes
#include <boost/locale.hpp>
#include <inputtino/input.hpp>
#include <libevdev/libevdev.h>

#ifdef SUNSHINE_BUILD_X11
  #include <X11/Xutil.h>
  #include <X11/extensions/XTest.h>
  #include <X11/keysym.h>
  #include <X11/keysymdef.h>
#endif

// local includes
#include "src/config.h"
#include "src/logging.h"
#include "src/platform/common.h"
#include "src/utility.h"

using namespace std::literals;

namespace platf {

  // ── XTest direct linking ───────────────────────────────────────────────────
  // libX11 and libXtst are linked directly at build time via CMake (PLATFORM_LIBRARIES).
  // No dynamic loading needed — just thin inline wrappers.
#ifdef SUNSHINE_BUILD_X11
  namespace x11 {
    inline Display *OpenDisplay(const char *name) {
      return XOpenDisplay(name);
    }
    inline int CloseDisplay(Display *d) {
      return XCloseDisplay(d);
    }
    inline void InitThreads() {
      XInitThreads();
    }
    inline int Flush(Display *d) {
      return XFlush(d);
    }
    inline KeyCode KeysymToKeycode(Display *d, KeySym ks) {
      return XKeysymToKeycode(d, ks);
    }

    namespace tst {
      inline int FakeMotionEvent(Display *d, int screen, int x, int y, unsigned long delay) {
        return XTestFakeMotionEvent(d, screen, x, y, delay);
      }
      inline int FakeRelativeMotionEvent(Display *d, int dx, int dy, unsigned long delay) {
        return XTestFakeRelativeMotionEvent(d, dx, dy, delay);
      }
      inline int FakeButtonEvent(Display *d, unsigned int button, Bool press, unsigned long delay) {
        return XTestFakeButtonEvent(d, button, press, delay);
      }
      inline int FakeKeyEvent(Display *d, unsigned int keycode, Bool press, unsigned long delay) {
        return XTestFakeKeyEvent(d, keycode, press, delay);
      }
    }  // namespace tst
  }  // namespace x11

  // ── Moonlight modcode → X11 KeySym table (from v0.23.1) ───────────────────
  constexpr auto UNKNOWN = 0;

  struct keycode_xtest_t {
    KeySym keysym;
  };

  static constexpr std::array<keycode_xtest_t, 0xE3>
  init_keysyms() {
    std::array<keycode_xtest_t, 0xE3> k {};
  #define KS(wincode, keysym) k[wincode] = keycode_xtest_t { keysym }

    KS(0x08, XK_BackSpace);  KS(0x09, XK_Tab);       KS(0x0C, XK_Clear);
    KS(0x0D, XK_Return);     KS(0x10, XK_Shift_L);   KS(0x11, XK_Control_L);
    KS(0x12, XK_Alt_L);      KS(0x13, XK_Pause);     KS(0x14, XK_Caps_Lock);
    KS(0x1B, XK_Escape);     KS(0x20, XK_space);     KS(0x21, XK_Page_Up);
    KS(0x22, XK_Page_Down);  KS(0x23, XK_End);       KS(0x24, XK_Home);
    KS(0x25, XK_Left);       KS(0x26, XK_Up);        KS(0x27, XK_Right);
    KS(0x28, XK_Down);       KS(0x2A, XK_Print);     KS(0x2C, XK_Sys_Req);
    KS(0x2D, XK_Insert);     KS(0x2E, XK_Delete);    KS(0x2F, XK_Help);
    KS(0x30, XK_0);  KS(0x31, XK_1);  KS(0x32, XK_2);  KS(0x33, XK_3);
    KS(0x34, XK_4);  KS(0x35, XK_5);  KS(0x36, XK_6);  KS(0x37, XK_7);
    KS(0x38, XK_8);  KS(0x39, XK_9);
    KS(0x41, XK_A);  KS(0x42, XK_B);  KS(0x43, XK_C);  KS(0x44, XK_D);
    KS(0x45, XK_E);  KS(0x46, XK_F);  KS(0x47, XK_G);  KS(0x48, XK_H);
    KS(0x49, XK_I);  KS(0x4A, XK_J);  KS(0x4B, XK_K);  KS(0x4C, XK_L);
    KS(0x4D, XK_M);  KS(0x4E, XK_N);  KS(0x4F, XK_O);  KS(0x50, XK_P);
    KS(0x51, XK_Q);  KS(0x52, XK_R);  KS(0x53, XK_S);  KS(0x54, XK_T);
    KS(0x55, XK_U);  KS(0x56, XK_V);  KS(0x57, XK_W);  KS(0x58, XK_X);
    KS(0x59, XK_Y);  KS(0x5A, XK_Z);
    KS(0x5B, XK_Meta_L);     KS(0x5C, XK_Meta_R);
    KS(0x60, XK_KP_0);  KS(0x61, XK_KP_1);  KS(0x62, XK_KP_2);
    KS(0x63, XK_KP_3);  KS(0x64, XK_KP_4);  KS(0x65, XK_KP_5);
    KS(0x66, XK_KP_6);  KS(0x67, XK_KP_7);  KS(0x68, XK_KP_8);
    KS(0x69, XK_KP_9);
    KS(0x6A, XK_KP_Multiply);   KS(0x6B, XK_KP_Add);
    KS(0x6C, XK_KP_Separator);  KS(0x6D, XK_KP_Subtract);
    KS(0x6E, XK_KP_Decimal);    KS(0x6F, XK_KP_Divide);
    KS(0x70, XK_F1);  KS(0x71, XK_F2);  KS(0x72, XK_F3);  KS(0x73, XK_F4);
    KS(0x74, XK_F5);  KS(0x75, XK_F6);  KS(0x76, XK_F7);  KS(0x77, XK_F8);
    KS(0x78, XK_F9);  KS(0x79, XK_F10); KS(0x7A, XK_F11); KS(0x7B, XK_F12);
    KS(0x7C, XK_F13); KS(0x7D, XK_F14); KS(0x7E, XK_F15); KS(0x7F, XK_F16);
    KS(0x80, XK_F17); KS(0x81, XK_F18); KS(0x82, XK_F19); KS(0x83, XK_F20);
    KS(0x84, XK_F21); KS(0x85, XK_F12); KS(0x86, XK_F23); KS(0x87, XK_F24);
    KS(0x90, XK_Num_Lock);    KS(0x91, XK_Scroll_Lock);
    KS(0xA0, XK_Shift_L);    KS(0xA1, XK_Shift_R);
    KS(0xA2, XK_Control_L);  KS(0xA3, XK_Control_R);
    KS(0xA4, XK_Alt_L);      KS(0xA5, XK_Alt_R);
    KS(0xBA, XK_semicolon);  KS(0xBB, XK_equal);
    KS(0xBC, XK_comma);      KS(0xBD, XK_minus);
    KS(0xBE, XK_period);     KS(0xBF, XK_slash);
    KS(0xC0, XK_grave);
    KS(0xDB, XK_braceleft);  KS(0xDC, XK_backslash);
    KS(0xDD, XK_braceright); KS(0xDE, XK_apostrophe);
    KS(0xE2, XK_backslash);
  #undef KS
    return k;
  }

  static constexpr auto keysyms = init_keysyms();
#endif  // SUNSHINE_BUILD_X11

  // ── input_raw_t ───────────────────────────────────────────────────────────
  using joypads_t = std::variant<inputtino::XboxOneJoypad, inputtino::SwitchJoypad, inputtino::PS5Joypad>;

  struct joypad_state {
    std::unique_ptr<joypads_t> joypad;
    gamepad_feedback_msg_t last_rumble;
    gamepad_feedback_msg_t last_rgb_led;
  };

  struct input_raw_t {
    input_raw_t():
        mouse(inputtino::Mouse::create({
          .name = "Mouse passthrough",
          .vendor_id = 0xBEEF,
          .product_id = 0xDEAD,
          .version = 0x111,
        })),
        keyboard(inputtino::Keyboard::create({
          .name = "Keyboard passthrough",
          .vendor_id = 0xBEEF,
          .product_id = 0xDEAD,
          .version = 0x111,
        })),
        gamepads(MAX_GAMEPADS) {
      if (!mouse) {
        BOOST_LOG(warning) << "Unable to create virtual mouse: " << mouse.getErrorMessage();
      }
      if (!keyboard) {
        BOOST_LOG(warning) << "Unable to create virtual keyboard: " << keyboard.getErrorMessage();
      }
    }

    ~input_raw_t() {
#ifdef SUNSHINE_BUILD_X11
      if (display) {
        x11::CloseDisplay(display);
        display = nullptr;
      }
#endif
    }

    inputtino::Result<inputtino::Mouse>    mouse;
    inputtino::Result<inputtino::Keyboard> keyboard;

    std::vector<std::shared_ptr<joypad_state>> gamepads;

#ifdef SUNSHINE_BUILD_X11
    Display *display = nullptr;
#endif
  };

  struct client_input_raw_t: public client_input_t {
    client_input_raw_t(input_t &input):
        touch(inputtino::TouchScreen::create({
          .name = "Touch passthrough",
          .vendor_id = 0xBEEF,
          .product_id = 0xDEAD,
          .version = 0x111,
        })),
        pen(inputtino::PenTablet::create({
          .name = "Pen passthrough",
          .vendor_id = 0xBEEF,
          .product_id = 0xDEAD,
          .version = 0x111,
        })) {
      global = (input_raw_t *) input.get();
      if (!touch) {
        BOOST_LOG(warning) << "Unable to create virtual touch screen: " << touch.getErrorMessage();
      }
      if (!pen) {
        BOOST_LOG(warning) << "Unable to create virtual pen tablet: " << pen.getErrorMessage();
      }
    }

    input_raw_t *global;

    inputtino::Result<inputtino::TouchScreen> touch;
    inputtino::Result<inputtino::PenTablet>   pen;
  };

  inline float deg2rad(float degree) {
    return degree * (M_PI / 180.f);
  }
}  // namespace platf
