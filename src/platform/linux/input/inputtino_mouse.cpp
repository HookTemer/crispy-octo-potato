/**
 * @file src/platform/linux/input/inputtino_mouse.cpp
 * @brief Definitions for inputtino mouse input handling.
 */
// lib includes
#include <boost/locale.hpp>
#include <inputtino/input.hpp>
#include <libevdev/libevdev.h>

// local includes
#include "inputtino_common.h"
#include "inputtino_mouse.h"
#include "src/config.h"
#include "src/logging.h"
#include "src/platform/common.h"
#include "src/utility.h"

using namespace std::literals;

namespace platf::mouse {

  void move(input_raw_t *raw, int deltaX, int deltaY) {
    if (raw->mouse) {
      (*raw->mouse).move(deltaX, deltaY);
      return;
    }
#ifdef SUNSHINE_BUILD_X11
    if (raw->display) {
      x11::tst::FakeRelativeMotionEvent(raw->display, deltaX, deltaY, CurrentTime);
      x11::Flush(raw->display);
    }
#endif
  }

  void move_abs(input_raw_t *raw, const touch_port_t &touch_port, float x, float y) {
    if (raw->mouse) {
      (*raw->mouse).move_abs(x, y, touch_port.width, touch_port.height);
      return;
    }
#ifdef SUNSHINE_BUILD_X11
    if (raw->display) {
      x11::tst::FakeMotionEvent(raw->display, -1, (int) x, (int) y, CurrentTime);
      x11::Flush(raw->display);
    }
#endif
  }

  void button(input_raw_t *raw, int button, bool release) {
    if (raw->mouse) {
      inputtino::Mouse::MOUSE_BUTTON btn_type;
      switch (button) {
        case BUTTON_LEFT:   btn_type = inputtino::Mouse::LEFT;   break;
        case BUTTON_MIDDLE: btn_type = inputtino::Mouse::MIDDLE; break;
        case BUTTON_RIGHT:  btn_type = inputtino::Mouse::RIGHT;  break;
        case BUTTON_X1:     btn_type = inputtino::Mouse::SIDE;   break;
        case BUTTON_X2:     btn_type = inputtino::Mouse::EXTRA;  break;
        default:
          BOOST_LOG(warning) << "Unknown mouse button: " << button;
          return;
      }
      if (release) {
        (*raw->mouse).release(btn_type);
      } else {
        (*raw->mouse).press(btn_type);
      }
      return;
    }
#ifdef SUNSHINE_BUILD_X11
    if (raw->display) {
      unsigned int x_button;
      switch (button) {
        case BUTTON_LEFT:   x_button = 1; break;
        case BUTTON_MIDDLE: x_button = 2; break;
        case BUTTON_RIGHT:  x_button = 3; break;
        default:            x_button = (button - 4) + 8; break;
      }
      if (x_button >= 1 && x_button <= 31) {
        x11::tst::FakeButtonEvent(raw->display, x_button, !release, CurrentTime);
        x11::Flush(raw->display);
      }
    }
#endif
  }

  void scroll(input_raw_t *raw, int high_res_distance) {
    if (raw->mouse) {
      (*raw->mouse).vertical_scroll(high_res_distance);
      return;
    }
#ifdef SUNSHINE_BUILD_X11
    if (raw->display) {
      int full_ticks = high_res_distance / 120;
      if (full_ticks == 0) return;
      const int btn = full_ticks > 0 ? 4 : 5;
      for (int i = 0; i < std::abs(full_ticks); i++) {
        x11::tst::FakeButtonEvent(raw->display, btn, true,  CurrentTime);
        x11::tst::FakeButtonEvent(raw->display, btn, false, CurrentTime);
      }
      x11::Flush(raw->display);
    }
#endif
  }

  void hscroll(input_raw_t *raw, int high_res_distance) {
    if (raw->mouse) {
      (*raw->mouse).horizontal_scroll(high_res_distance);
      return;
    }
#ifdef SUNSHINE_BUILD_X11
    if (raw->display) {
      int full_ticks = high_res_distance / 120;
      if (full_ticks == 0) return;
      const int btn = full_ticks > 0 ? 6 : 7;
      for (int i = 0; i < std::abs(full_ticks); i++) {
        x11::tst::FakeButtonEvent(raw->display, btn, true,  CurrentTime);
        x11::tst::FakeButtonEvent(raw->display, btn, false, CurrentTime);
      }
      x11::Flush(raw->display);
    }
#endif
  }

  util::point_t get_location(input_raw_t *raw) {
    return {0, 0};
  }
}  // namespace platf::mouse
