/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"

/* 
 * Implement of auto mouse layer 
 * According to https://github.com/kamiichi99/keyball/blob/main/qmk_firmware/keyboards/keyball/keyball44/keymaps/kamidai/keymap.c by kamiichi99
*/

enum custom_keycodes {
  KC_MY_BTN1 = KEYBALL_SAFE_RANGE,
  KC_MY_BTN2,
  KC_MY_BTN3,
};

enum click_state {
  NONE = 0,
  WAITING, // Wait for mouse layer to activate.
  CLICKABLE, // Mouse layer is enabled to take click input
  CLICKING,
};

enum click_state state; // Current click input reception status
uint16_t click_timer; // Time to determine tha state of the system

uint16_t to_reset_time = 800; // For this number of seconds (milliseconds), the click layer is disabled if in CLICKABLE state

const int16_t to_clickable_movement = 0; // The movement distance to enable click layer 
const uint16_t click_layer = 3; // Layer enabled when the mouse input is enabled

int16_t mouse_record_threshold = 30; // Number of frames in which the pointer movement is temporarily recorded.
int16_t mouse_move_count_ratio = 5;  // The coefficient of the moving frame when replaying the pointer movement.

int16_t mouse_movement;

// Enable layers for clicks
void enable_mouse_layer(void) {
  layer_on(click_layer);
  state = CLICKABLE;
}

// Disable layers for clicks
void disable_mouse_layer(void) {
  layer_off(click_layer);
  state = WAITING;
}

// Function that returns absolute numbers
int16_t abs16(int16_t x) {
  return x < 0 ? -x : x;
}

// Functions that returns the sign of the number
int16_t sign16(int16_t x) {
  return x < 0 ? -1 : 1;
}

// Function that returns is currently clickable
bool is_clickable(void) {
  return state == CLICKABLE || state == CLICKING;
}

//
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case KC_MY_BTN1:
    case KC_MY_BTN2:
    case KC_MY_BTN3: {
      report_mouse_t currentReport = pointing_device_get_report();

      // Which bits are to be targeted
      uint8_t btn = 1 << (keycode - KC_MY_BTN1);

      if (record->event.pressed) {
        currentReport.buttons |= btn;
        state = CLICKING;
      } else {
        currentReport.buttons &= ~btn;
        enable_mouse_layer();
      }

      pointing_device_set_report(currentReport);
      pointing_device_send();
      return false;
    }

    default:
      if (record->event.pressed) {
        disable_mouse_layer();
      }
  }
  return true;
}


report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
  int16_t current_x = mouse_report.x;
  int16_t current_y = mouse_report.y;

  if (current_x != 0 || current_y != 0) {

    switch (state) {
      case CLICKABLE:
        click_timer = timer_read();
        break;

      case CLICKING:
        break;

      case WAITING:
        mouse_movement += abs16(current_x) + abs16(current_y);

        if (mouse_movement >= to_clickable_movement) {
          mouse_movement = 0;
          enable_mouse_layer();
        }
        break;

      default:
        click_timer = timer_read();
        state = WAITING;
        mouse_movement = 0;
    }
  } else {
    switch (state) {
      case CLICKING:
        break;

      case CLICKABLE:
        if (timer_elapsed(click_timer) > to_reset_time) {
          disable_mouse_layer();
        }
        break;

      case WAITING:
        if (timer_elapsed(click_timer) > 50) {
          mouse_movement = 0;
          state = NONE;
        }
        break;

      default:
        mouse_movement = 0;
        state = NONE; 
    }
  }

  mouse_report.x = current_x;
  mouse_report.y = current_y;

  return mouse_report;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [0] = LAYOUT_universal(
    MT(KC_LEFT_CTRL, KC_TAB)   , KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                                        KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     , KC_DEL   ,
    KC_LSFT   , KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                                        KC_H     , KC_J     , KC_K     , KC_L     , KC_SCLN  , S(KC_7)  ,
    KC_LALT  , KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                                        KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  , KC_INT1  ,
              KC_NO,KC_NO,LT(3, KC_SPC)     ,KC_LEFT_GUI,LT(3,KC_LNG2),                  KC_LNG1,TT(2), KC_NO,     KC_NO  , TT(3)
  ),

  [1] = LAYOUT_universal(
    SSNP_FRE ,  KC_F1   , KC_F2    , KC_F3   , KC_F4    , KC_F5    ,                                         KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   , KC_F11   ,
    SSNP_VRT ,  _______ , _______  , KC_UP   , KC_ENT   , KC_DEL   ,                                         KC_PGUP  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_BTN3  , KC_F12   ,
    SSNP_HOR ,  _______ , KC_LEFT  , KC_DOWN , KC_RGHT  , KC_BSPC  ,                                         KC_PGDN  , KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  , _______  ,
                  _______  , _______ , _______  ,         _______  , _______  ,                   _______  , _______  , _______       , _______  , _______
  ),

  [2] = LAYOUT_universal(
    _______  ,S(KC_QUOT), KC_7     , KC_8    , KC_9     , S(KC_8)  ,                                         S(KC_9)  , S(KC_1)  , S(KC_6)  , KC_LBRC  , S(KC_4)  , _______  ,
    _______  ,S(KC_SCLN), KC_4     , KC_5    , KC_6     , KC_RBRC  ,                                         KC_NUHS  , KC_MINS  , S(KC_EQL), S(KC_3)  , KC_QUOT  , S(KC_2)  ,
    _______  ,S(KC_MINS), KC_1     , KC_2    , KC_3     ,S(KC_RBRC),                                        S(KC_NUHS),S(KC_INT1), KC_EQL   ,S(KC_LBRC),S(KC_SLSH),S(KC_INT3),
                  KC_0     , KC_DOT  , _______  ,         _______  , _______  ,                   KC_DEL   , _______  , _______       , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  , _______  ,                                        RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN , RGB_M_K  ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , _______  , SCRL_DVI ,                                        RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW , _______  , _______  ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , _______  , SCRL_DVD ,                                        CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , _______  , KBC_SAVE ,
                  QK_BOOT  , KBC_RST  , _______  ,        _______  , _______  ,                   _______  , _______  , _______       , KBC_RST  , QK_BOOT
  ),
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is not mouse layer
    keyball_set_scroll_mode(get_highest_layer(state) != 3);
    return state;
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
